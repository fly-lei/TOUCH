/* modbus_app.c */
#include "qpc.h"
#include "signals.h"
#include "usart.h"
/* 1. 极其霸道的 20KB 全局寄存器池 (覆盖 0~10000 地址) */
uint16_t g_mb_regs[10001] = {0};

/* 2. 定义 Modbus 活动对象 */
typedef struct {
    QActive super;
} Modbus;
extern UART_HandleTypeDef huart3;
static Modbus l_modbus; /* 实例对象 */
QActive * const AO_Modbus = &l_modbus.super;
uint16_t MB_CRC16(const uint8_t *puchMsg, uint16_t usDataLen);
/* 状态函数声明 */
static QState Modbus_initial(Modbus * const me, QEvt const * const e);
static QState Modbus_idle(Modbus * const me, QEvt const * const e);

/* 3. 构造函数 */
void Modbus_ctor(void) {
    QActive_ctor(&l_modbus.super, Q_STATE_CAST(&Modbus_initial));
}

static QState Modbus_initial(Modbus * const me, QEvt const * const e) {
    /* 初始化串口外设，准备接收 (通常在外面 HAL_UART_Receive_DMA) */
    return Q_TRAN(&Modbus_idle);
}

/* ========================================================= */
/* 核心状态：待命与解析解析 (Idle)                           */
/* ========================================================= */
static QState Modbus_idle(Modbus * const me, QEvt const * const e) {
    switch (e->sig) {

        /* 收到完整报文！开始疯狂解析 */
        case MB_RX_FRAME_SIG: {
        	/* 1. 极其严谨：所有变量声明必须顶格放在大括号最前面！ */
        	            MbRxEvt const *pe = (MbRxEvt const *)e;
        	            uint8_t slave_id;
        	            uint8_t func_code;
        	            uint16_t crc_calc;
        	            uint16_t crc_recv;

        	            /* 必须加 static！保证函数结束后，内存依然存活供 DMA 慢慢搬运！ */
        	            static uint8_t tx_buf[256];
        	            uint16_t tx_len = 0;

        	            /* 2. 防爆甲：如果收到的全是噪点碎片(小于4字节)，直接丢弃，防止数组越界死机！ */
        	            if (pe->len < 4) {
        	                return Q_HANDLED();
        	            }

        	            /* 3. 变量赋值与 CRC 计算 */
        	            slave_id  = pe->frame[0];
        	            func_code = pe->frame[1];
        	            crc_calc  = MB_CRC16(pe->frame, pe->len - 2);
        	            /* Modbus 协议规定 CRC 低位在前，高位在后 */
        	            crc_recv  = pe->frame[pe->len - 2] | (pe->frame[pe->len - 1] << 8);

        	            /* 👇 4. 架构师的照妖镜 (QSPY 探针) 👇 */
        	#ifdef Q_SPY
        	            QS_BEGIN_ID(QS_USER, 0)
        	                QS_STR("ID:"); QS_U8(1, slave_id);
        	                QS_STR(" FC:"); QS_U8(1, func_code);
        	                QS_STR(" Calc:"); QS_U16(4, crc_calc);
        	                QS_STR(" Recv:"); QS_U16(4, crc_recv);
        	            QS_END()
        	#endif

        	            /* 5. 第一堵墙：站号过滤 (假设本机 ID 是 1) */
        	            if (slave_id != 1) {
        	                return Q_HANDLED();
        	            }

        	            /* 6. 第二堵墙：CRC 校验 (如果您只是想跑通，可以先把这行注释掉！) */
        	            if (crc_calc != crc_recv) {
        	                return Q_HANDLED();
        	            }

            /* 3. 核心业务：功能码分发路由 */
            switch (func_code) {
                case 0x03: { /* 读保持寄存器 */
                    uint16_t start_addr = (pe->frame[2] << 8) | pe->frame[3];
                    uint16_t reg_cnt    = (pe->frame[4] << 8) | pe->frame[5];

                    /* 防越界保护 */
                    if (start_addr + reg_cnt > 10001) {
                        /* 可以在这组装 Modbus 异常报文 (0x83) 并发送 */
                        break;
                    }

                    /* 组装正常回包 */
                    tx_buf[0] = slave_id;
                    tx_buf[1] = 0x03;
                    tx_buf[2] = reg_cnt * 2; /* 字节数 */
                    tx_len = 3;
                    for (uint16_t i = 0; i < reg_cnt; i++) {
                        tx_buf[tx_len++] = g_mb_regs[start_addr + i] >> 8;   /* 高位 */
                        tx_buf[tx_len++] = g_mb_regs[start_addr + i] & 0xFF; /* 低位 */
                    }
                    break;
                }
                case 0x06: { /* 写单个寄存器 */
                    uint16_t reg_addr = (pe->frame[2] << 8) | pe->frame[3];
                    uint16_t reg_val  = (pe->frame[4] << 8) | pe->frame[5];

                    if (reg_addr <= 10000) {
                        g_mb_regs[reg_addr] = reg_val; /* 瞬间写入 20KB 内存池！ */

                        /* 06 功能码的回包就是原样返回 */
                        for(tx_len=0; tx_len < pe->len - 2; tx_len++) {
                            tx_buf[tx_len] = pe->frame[tx_len];
                        }
                    }
                    break;
                }
                case 0x10: { /* 写多个寄存器 */
                    uint16_t start_addr = (pe->frame[2] << 8) | pe->frame[3];
                    uint16_t reg_cnt    = (pe->frame[4] << 8) | pe->frame[5];
                    uint8_t  byte_cnt   = pe->frame[6];

                    if (start_addr + reg_cnt <= 10001 && byte_cnt == reg_cnt * 2) {
                        for (uint16_t i = 0; i < reg_cnt; i++) {
                            uint16_t val = (pe->frame[7 + i*2] << 8) | pe->frame[8 + i*2];
                            g_mb_regs[start_addr + i] = val; /* 批量写入内存池 */
                        }
                        /* 组装回包：返回写入的起始地址和数量 */
                        tx_buf[0] = slave_id;
                        tx_buf[1] = 0x10;
                        tx_buf[2] = pe->frame[2]; tx_buf[3] = pe->frame[3];
                        tx_buf[4] = pe->frame[4]; tx_buf[5] = pe->frame[5];
                        tx_len = 6;
                    }
                    break;
                }
                default:
                    /* 不支持的功能码，直接丢弃或回异常 */
                    break;
            }

            /* 4. 打上 CRC 烙印，并通过 DMA 发射出去！ */
            /* 在 modbus_app.c 的 Modbus_idle 状态中 */
                        if (tx_len > 0) {
                            uint16_t tx_crc = MB_CRC16(tx_buf, tx_len);
                            tx_buf[tx_len++] = tx_crc & 0xFF;
                            tx_buf[tx_len++] = tx_crc >> 8;

                            /* 👇 改用新的串口句柄进行 DMA 发送 👇 */

                            HAL_UART_Transmit_DMA(&huart3, tx_buf, tx_len);
                           // HAL_UART_Transmit(&huart3, tx_buf, tx_len, 100);
                        }

            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

/* 标准 Modbus CRC16 校验算法 (极速查表法太占内存，这里用极其优雅的纯算公式) */
uint16_t MB_CRC16(const uint8_t *puchMsg, uint16_t usDataLen) {
    uint16_t crc16 = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < usDataLen; i++) {
        crc16 ^= puchMsg[i];
        for (j = 0; j < 8; j++) {
            if (crc16 & 0x0001) {
                crc16 = (crc16 >> 1) ^ 0xA001;
            } else {
                crc16 >>= 1;
            }
        }
    }
    return crc16;
}
