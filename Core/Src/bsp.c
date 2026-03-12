/* Core/Src/bsp.c */
#include "qpc.h"
#include "blinky.h"
#include "stm32f4xx_hal.h"
/* USER CODE BEGIN 4 */
/* 映射公式: (原始值 - 最小值) * 目标范围 / (最大值 - 最小值) */
// 这里假设 X 和 Y 的原始值范围大约在 200 到 3800 之间（具体需看您的打印）
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
/* QV 框架强制要求的空闲处理回调函数 */
//void QV_onIdle(void) {
//    /* 【生命之源】告诉内核：我已准备好接收新的中断！ */
//	QF_INT_ENABLE();
//
//    /* 注意：在完全调试通之前，这里绝对不要加 __WFI() 或任何代码！ */
//}
void QV_onIdle(void) {
    QF_INT_ENABLE(); // 必须开中断
    UART_Log("Enter OFF State\r\n");
    // 暂时把里面发数据的代码全注释掉，只留下睡觉指令
    __WFI();
}
/* USER CODE END 4 */

// 框架启动回调
void QF_onStartup(void) {
    // 这里可以放 SysTick_Config 等，但 CubeMX 已经在 main 里配好了 HAL_InitTick
}
/* 软件延时函数，用于在中断关闭时提供延时 */
void soft_delay(uint32_t ms) {
    // 假设主频在 84~168MHz 之间，简单估算循环次数
    uint32_t count = ms * 40000;
    for (volatile uint32_t i = 0; i < count; i++) { }
}/* USER CODE BEGIN 4 */
#include <stdio.h>

/* QP框架的终极崩溃回调函数 */
void Q_onAssert(char const * const module, int loc) {

    /* 1. 关门打狗：强行关闭所有硬件中断，防止任何外设再来捣乱 */
    __disable_irq();

    /* 2. 准备遗言：把凶手的模块名和行号写进内存 */
    char buf[128];
    sprintf(buf, "\r\n\r\n!!! [FATAL ASSERT INSIDE QF_RUN] !!!\r\nModule: %s\r\nID/Line: %d\r\n\r\n", module, loc);

    /* 3. 强行发报：抛弃所有 HAL 库，直接向 USART1 寄存器里砸数据！ */
    /* ⚠️ 假设您用的是 USART1，如果是 USART2 请改掉 */
    for (int i = 0; buf[i] != '\0'; i++) {
        /* 死等发送数据寄存器空 (TXE) */
        while ((USART1->SR & (1 << 7)) == 0) {
        }
        /* 砸入一个字节 */
        USART1->DR = buf[i];
    }

    /* 4. 死不瞑目：发完遗言后，让系统永远停在这里等待您收尸 */
    while (1) {
    }
}

/* USER CODE END 4 */
/* USER CODE BEGIN 4 */
/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

    if (GPIO_Pin == GPIO_PIN_1) {


            /* 【完美吸收官方写法】：使用官方宏初始化静态事件！杜绝结构体内存未对齐问题 */
            static QEvt const btnEvt = QEVT_INITIALIZER(BTN_PA1_SIG);

            /* 安全投递，第三个参数填 0U (无 QP Spy 追踪) 即可 */
            QACTIVE_POST(AO_Blinky, &btnEvt, 0U);


        /* 彻底清除中断标志，并且做一次总线同步（达到官方 ERRATUM 补丁的效果） */

    }
    if (GPIO_Pin == GPIO_PIN_4) {
           static uint32_t last_btn_time = 0;
           uint32_t current_time = HAL_GetTick();

           /* 200ms 防抖锁：防住物理按键的弹片抖动 */
           if ((current_time - last_btn_time) > 200) {
               last_btn_time = current_time;

               //static QEvt const touchEvt = { TOUCH_SIG, 0U, 0U };
               static QEvt const touchEvt =QEVT_INITIALIZER(TOUCH_SIG);
               QACTIVE_POST(AO_Blinky, &touchEvt, 0U);

           }

       }

}
/* USER CODE BEGIN 4 */

#ifdef Q_SPY

/* 1. 给 QS 提供时间戳 (毫秒级) */
QSTimeCtr QS_onGetTime(void) {
    return (QSTimeCtr)HAL_GetTick();
}

/* 2. QS 启动配置 */
void QS_onStartup(void const *arg) {
    static uint8_t qsTxBuf[1024]; /* 必须给 QS 分配一块内存做发送缓存 */
    QS_initBuf(qsTxBuf, sizeof(qsTxBuf));

    /* 开启最高级别的监视：监控所有状态跳转和队列动作！ */
    QS_GLB_FILTER(QS_ALL_RECORDS);

    /* 【字典翻译】：教 QS 认识您的信号名字，否则电脑上只会显示冰冷的数字 */
    QS_SIG_DICTIONARY(TIMEOUT_SIG, (void *)0);
    QS_SIG_DICTIONARY(TOUCH_SIG,   (void *)0);
    QS_SIG_DICTIONARY(BTN_PA1_SIG, (void *)0);

    /* 教 QS 认识您的状态机名字 */
    QS_OBJ_DICTIONARY(AO_Blinky);
    QS_FUN_DICTIONARY(&Blinky_on);
    QS_FUN_DICTIONARY(&Blinky_off);
}
/* USER CODE BEGIN 4 */

void QS_onFlush(void) {
    uint16_t b;
    /* ⚠️ 此时全局中断已关，SysTick 已死，绝对不能用 HAL_UART_Transmit！ */
    while ((b = QS_getByte()) != QS_EOD) {
        /* 死等发送寄存器(SR)的 TXE 标志位变为空 */
        while ((USART1->SR & USART_SR_TXE) == 0) {
        }
        /* 把字节直接砸进数据寄存器(DR) */
        USART1->DR = (uint8_t)b;
    }
}

/* USER CODE END 4 */

#endif /* Q_SPY */

/* USER CODE END 4 */


