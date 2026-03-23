/* flight_recorder.h */
#ifndef FLIGHT_RECORDER_H
#define FLIGHT_RECORDER_H

#include <stdint.h>
#include "qpc.h"

/* 架构师配置区 */
#define FR_BUFFER_SIZE       64          /* 在 RAM 里保留死机前最后的 64 步轨迹 */
#define PANIC_SECTOR_ADDR    0x001000    /* W25Q128 的最后一个扇区 (第 4095 扇区) */
#define BLACKBOX_MAGIC_VALID 0xDEADBEEF  /* 有效死亡日志的魔数标记 */

/* 极其紧凑的单条日志格式 (12 字节) */
typedef struct {
    uint32_t timestamp;  /* 滴答时间 */
    uint32_t state_id;   /* 状态机代号 (您可以自己定义一套枚举，比如 1代表温控，2代表显示) */
    uint16_t event_sig;  /* 导致跳转的信号 (如 Q_ENTRY_SIG, TICK_SIG) */
    float    key_value;  /* 关键物理量 (如 PID 误差、温度) */
} FR_Record_t;

/* 飞行记录仪大本营 (占用 RAM 大约 768 字节) */
typedef struct {
    uint32_t magic;                    /* 魔数标记 */
    uint32_t head;                     /* 环形缓冲区的写入指针 */
    char     death_cause[16];          /* 临终遗言：肇事模块或原因 */
    uint32_t death_location;           /* 临终代码行号 */
    FR_Record_t records[FR_BUFFER_SIZE]; /* 环形轨迹区 */
} Flight_Recorder_t;

/* 暴露给全局的写入接口 */
void FR_Log_Event(uint32_t state, uint16_t sig, float val);
void Check_And_Print_Panic_Dump(void);

#endif /* FLIGHT_RECORDER_H */
