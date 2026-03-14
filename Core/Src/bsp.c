
#include "usart.h"
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
    QF_INT_ENABLE();

#ifdef Q_SPY
    uint8_t const *block;
    uint16_t len;
    extern UART_HandleTypeDef huart1;
   // char aaa[]="hello wo";
    //HAL_UART_Transmit(&huart1, (uint8_t *)aaa, 5, 100);
    /* ⚠️ 极其关键的 API 升级：指针和长度的位置互换了！ */
    /* block 接收返回值，&len 作为参数传进去 */
    while ((block = QS_getBlock(&len)) != (uint8_t const *)0) {

        HAL_UART_Transmit(&huart1, (uint8_t *)block, len, 100);
    }
#else
    __WFI();
#endif
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
           if ((current_time - last_btn_time) > 100) {
               last_btn_time = current_time;
               static QEvt const touchEvt =QEVT_INITIALIZER(TOUCH_SIG);
                   	 QACTIVE_POST(AO_Blinky, &touchEvt, 0U);

               //static QEvt const touchEvt = { TOUCH_SIG, 0U, 0U };

           }

       }

}
/* USER CODE BEGIN 4 */

#ifdef Q_SPY

/* 1. 给 QS 提供时间戳 (毫秒级) */
QSTimeCtr QS_onGetTime(void) {
    return (QSTimeCtr)HAL_GetTick();
}

/* 2. QS 启动配置 *//* 1. 把 void 改成 uint8_t */
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsTxBuf[1024];
    QS_initBuf(qsTxBuf, sizeof(qsTxBuf));

    QS_GLB_FILTER(QS_ALL_RECORDS);
    QS_GLB_FILTER(-QS_QF_TICK);
    QS_SIG_DICTIONARY(TIMEOUT_SIG, (void *)0);
    QS_SIG_DICTIONARY(TIMEOUT1_SIG, (void *)0);
    QS_SIG_DICTIONARY(TOUCH_SIG,   (void *)0);
    QS_SIG_DICTIONARY(BTN_PA1_SIG, (void *)0);

    /* 2. 加上这句，告诉框架初始化成功！ */
    return 1U;
}
void QS_onFlush(void) {
    uint16_t b;
    /* ⚠️ 此时全局中断已关，SysTick 已死，绝对不能用 HAL_UART_Transmit！ */
    /* 修复：将 QS_EOD 替换为 0xFFFF (或新版本的 QS_EOF) */
    while ((b = QS_getByte()) != 0xFFFF) {
        /* 死等发送寄存器(SR)的 第7位 (TXE) 变为空 */
        while ((USART1->SR & (1 << 7)) == 0) {
        }
        /* 把字节直接砸进数据寄存器(DR) */
        USART1->DR = (uint8_t)b;
    }
}

/* USER CODE END 4 */

/* USER CODE END 4 */

#endif /* Q_SPY */

/* USER CODE END 4 */


