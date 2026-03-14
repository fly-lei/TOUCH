/*
 * blinky.c
 *
 *  Created on: Mar 11, 2026
 *      Author: 14019
 */


#include "blinky.h"
#include "stm32f4xx_hal.h" // 包含 HAL 库以控制 GPIO
#include "main.h"          // 包含 CubeMX 生成的引脚宏
#include "qpc.h"
/* 假设系统每秒 1000 个节拍 */
#define BSP_TICKS_PER_SEC 1000U
#define TICKS_PER_MS(ms_) (((ms_) * BSP_TICKS_PER_SEC) / 1000U)
// ... 您的其他包含 ...

/* 声明外部的清屏函数和颜色 */
extern void LCD_Clear(uint16_t color);

#define RED   0xF800
#define BLUE  0x001F
#define T_IRQ_READ()  HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)
extern void UART_Log(const char *fmt, ...);
extern uint16_t XPT2046_Read_Adc(uint8_t cmd);
#define RAW_X_MIN 200
#define RAW_X_MAX 3800
#define RAW_Y_MIN 200
#define RAW_Y_MAX 3800

uint16_t Calcu_LCD_X(uint16_t raw_x) {
    if (raw_x < RAW_X_MIN) return 0;
    return (uint32_t)(raw_x - RAW_X_MIN) * 240 / (RAW_X_MAX - RAW_X_MIN);
}

uint16_t Calcu_LCD_Y(uint16_t raw_y) {
    if (raw_y < RAW_Y_MIN) return 0;
    return (uint32_t)(raw_y - RAW_Y_MIN) * 320 / (RAW_Y_MAX - RAW_Y_MIN);
}

/* 1. 定义 Blinky 活动对象结构体 */
typedef struct {
    QActive super;       // 继承 QActive 基类
    QTimeEvt timeEvt;    // 定义一个私有时间事件 (定时器)
    int32_t press_count;
} Blinky;

/* 2. 声明状态处理函数 */
static QState Blinky_initial(Blinky * const me, QEvt const * const e);
static QState Blinky_off    (Blinky * const me, QEvt const * const e);
static QState Blinky_on     (Blinky * const me, QEvt const * const e);

/* 3. 实例化活动对象 */
static Blinky l_blinky; // 静态实例
QActive * const AO_Blinky = &l_blinky.super; // 供外部使用的指针

/* 4. 构造函数实现 */
void Blinky_ctor(void) {
    Blinky *me = &l_blinky;
    // 初始化基类状态机，初始状态指向 Blinky_initial
    QActive_ctor(&me->super, Q_STATE_CAST(&Blinky_initial));

    // 初始化时间事件 (绑定到当前活动对象，发送 TIMEOUT_SIG 信号)
    QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT1_SIG, 0U);
}

/* 5. 初始状态 (系统刚启动时进入) *//* 在 blinky.c 文件中 */

QState Blinky_initial(Blinky * const me, QEvt const * const e) {
    (void)e; /* 避免编译器报 unused 警告 */

#ifdef Q_SPY
    /* 1. 注册对象自己 (me 就是 AO_Blinky 的指针) */
    QS_OBJ_DICTIONARY(me);
    /* 2. 注册自己的状态函数 (此时编译器绝对认识它们) */
    QS_FUN_DICTIONARY(&Blinky_on);
    QS_FUN_DICTIONARY(&Blinky_off);
#endif

    /* 3. 给您的定时器上发条 (之前咱们讨论过的) */
    QTimeEvt_armX(&me->timeEvt, TICKS_PER_MS(5000U), TICKS_PER_MS(5000U));
    me->press_count = 15;
    return Q_TRAN(&Blinky_on);
}

QState Blinky_off(Blinky * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
        	//QTimeEvt_armX(&me->timeEvt, TICKS_PER_MS(500U), 0U);
            //UART_Log("Enter OFF State\r\n");
            return Q_HANDLED(); /* 极其重要 */
        }


        case BTN_PA1_SIG: {


        	            /* 2. 炫技时刻：用 QSPY 把数字打印到上位机！ */
        	#ifdef Q_SPY
        	            /* QS_USER 是留给用户的自定义打印通道 */
        	            QS_BEGIN_ID(QS_USER, me->super.prio)
        	                QS_STR("Button Pressed! Total Count: ");
        	                QS_U32(4, me->press_count); /* 打印这个 4 字节的变量 */
        	            QS_END()
        	#endif
        	            /* 处理完毕，告诉框架不需要其他操作了 */
        	                me->press_count--;
        	                        	                        	if(me->press_count<10){
        	                        	                        		return Q_TRAN(&Blinky_on);
        	                        	                        	}else{
        	                        	            return Q_HANDLED();
        	                        	                        	}


                }
        case Q_EXIT_SIG: {
        	//QTimeEvt_disarm(&me->timeEvt);
            return Q_HANDLED(); /* 极其重要 */
        }
        case TOUCH_SIG: {
           // uint16_t raw_x = XPT2046_Read_Adc(0xD0);
           // uint16_t raw_y = XPT2046_Read_Adc(0x90);
            //UART_Log("Touch OFF! X:%04d, Y:%04d\r\n", raw_x, raw_y);

            /* 在 OFF 状态收到触摸，无条件跳到 ON 状态！ */
        	//return Q_HANDLED();
        	return Q_TRAN(&Blinky_on);
        }
        case TIMEOUT1_SIG: {
                   // uint16_t raw_x = XPT2046_Read_Adc(0xD0);
                   // uint16_t raw_y = XPT2046_Read_Adc(0x90);
                    //UART_Log("Touch OFF! X:%04d, Y:%04d\r\n", raw_x, raw_y);

                    /* 在 OFF 状态收到触摸，无条件跳到 ON 状态！ */
                    return Q_TRAN(&Blinky_on);
                }
    }
    return Q_SUPER(&QHsm_top);
}
QState Blinky_on(Blinky * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
        	//QTimeEvt_armX(&me->timeEvt, TICKS_PER_MS(500U), 0U);
           // UART_Log("Enter ON State\r\n");
            return Q_HANDLED(); /* 极其重要，绝不能漏 */
        }
        case TIMEOUT1_SIG: {
                         //  uint16_t raw_x = XPT2046_Read_Adc(0xD0);
                         //  uint16_t raw_y = XPT2046_Read_Adc(0x90);
                           //UART_Log("Touch OFF! X:%04d, Y:%04d\r\n", raw_x, raw_y);

                           /* 在 OFF 状态收到触摸，无条件跳到 ON 状态！ */
                           return Q_TRAN(&Blinky_off);
                       }
        case BTN_PA1_SIG: {


        	            /* 2. 炫技时刻：用 QSPY 把数字打印到上位机！ */
        	#ifdef Q_SPY
        	            /* QS_USER 是留给用户的自定义打印通道 */
        	            QS_BEGIN_ID(QS_USER, me->super.prio)
        	                QS_STR("Button Pressed! Total Count: ");
        	                QS_U32(4, me->press_count); /* 打印这个 4 字节的变量 */
        	            QS_END()
        	#endif
        	            /* 处理完毕，告诉框架不需要其他操作了 */
        	                me->press_count++;
        	                        	if(me->press_count>20){
        	                        		return Q_TRAN(&Blinky_off);
        	                        	}else{
        	            return Q_HANDLED();
        	                        	}
                }

        case Q_EXIT_SIG: {
        	//QTimeEvt_disarm(&me->timeEvt);
            return Q_HANDLED(); /* 极其重要，绝不能漏 */
        }
        case TOUCH_SIG: {
          //  uint16_t raw_x = XPT2046_Read_Adc(0xD0);
          //  uint16_t raw_y = XPT2046_Read_Adc(0x90);
           // UART_Log("Touch ON! X:%04d, Y:%04d\r\n", raw_x, raw_y);

            /* 在 ON 状态收到触摸，无条件跳到 OFF 状态！不用 if 判断！ */
        	 // return Q_HANDLED();
        	  return Q_TRAN(&Blinky_off);
        }
    }
    return Q_SUPER(&QHsm_top);
}




