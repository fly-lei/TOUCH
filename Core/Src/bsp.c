
#include "usart.h"
#include "blinky.h"
#include "stm32f4xx_hal.h"
#include "signals.h"
#include "ILI9341.h"

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
void QS_onFlush(void);
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

/* QP框架的终极崩溃回调函数 *//* USER CODE BEGIN 4 */
/* USER CODE BEGIN 4 */
/* USER CODE BEGIN 4 */

#ifdef Q_SPY

/* ... 您之前的 QS_onStartup 和 QS_onFlush ... */

/* 满足链接器的要求：QS 的关机清理回调函数 */
void QS_onCleanup(void) {
    /* 裸机系统永远不会正常退出，所以这里什么都不用写，留空即可 */
}

#endif /* Q_SPY */

/* USER CODE END 4 */
/* 新版框架的崩溃回调叫 Q_onError，且进入时框架已经自动关了中断 */
void Q_onError(char const * const module, int loc) {

#ifdef Q_SPY
    /* 1. 生成标准的二进制崩溃日志！
       (第三个参数 10000U 是给底层的延时缓冲，防止串口发太快导致丢包) */
    QS_ASSERTION(module, loc, 10000U);

    /* 2. 强行把刚才生成的崩溃遗言，通过我们在 bsp 里写的寄存器逻辑砸出去！ */
    QS_onFlush();
#endif

    /* 3. 彻底死机，等待您收尸复位 */
    while (1) {
    }
}

/* USER CODE END 4 */

/* USER CODE END 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {



	    /* 1. 全局防抖锁：防止机械按键弹片抖动导致一次按下触发十几次中断 */
	    static uint32_t last_btn_time = 0;
	    uint32_t current_time = HAL_GetTick();

	    if ((current_time - last_btn_time) > 200) { /* 200ms 防抖 */

	        uint8_t valid_press = 0;

	        /* 2. 物理引脚映射到 UI 导航信号 */
	        if (GPIO_Pin == GPIO_PIN_0) { /* 假设 PA0 是“向下切换”按键 */
	            static QEvt const navDownEvt = QEVT_INITIALIZER(NAV_DOWN_SIG);
	            QACTIVE_POST(AO_Gui, &navDownEvt, 0U); /* 直接发给 GUI 部门！ */
	            valid_press = 1;
	        }
	        else if (GPIO_Pin == GPIO_PIN_1) { /* 假设 PA1 依然是“计分按键” */
	            static QEvt const btnEvt = QEVT_INITIALIZER(BTN_PRESSED_SIG);
	            QACTIVE_POST(AO_App, &btnEvt, 0U);     /* 发给大脑部门去算数！ */
	            valid_press = 1;
	        }
	        else if (GPIO_Pin == GPIO_PIN_4) { /* 假设 PE2 是“确认进入”按键 */
	            static QEvt const navEnterEvt = QEVT_INITIALIZER(NAV_ENTER_SIG);
	            QACTIVE_POST(AO_Gui, &navEnterEvt, 0U);
	            valid_press = 1;
	        }
	        else if (GPIO_Pin == GPIO_PIN_3) { /* 假设 PE3 是“返回/退出”按键 */
	            static QEvt const navBackEvt = QEVT_INITIALIZER(NAV_BACK_SIG);
	            QACTIVE_POST(AO_Gui, &navBackEvt, 0U);
	            valid_press = 1;
	        }

	        /* 如果确实按下了有效按键，更新防抖时间 */
	        if (valid_press) {
	            last_btn_time = current_time;
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
    /* ... 前面的初始化 ... */
	 static uint8_t qsTxBuf[1024];
	    QS_initBuf(qsTxBuf, sizeof(qsTxBuf));

	    QS_GLB_FILTER(QS_ALL_RECORDS);
	    QS_GLB_FILTER(-QS_QF_TICK);
    /* 原来的字典 */
    QS_SIG_DICTIONARY(BTN_PRESSED_SIG, (void *)0);
    QS_SIG_DICTIONARY(TOUCH_DETECTED_SIG, (void *)0);
    QS_SIG_DICTIONARY(UI_UPDATE_COUNT_SIG, (void *)0);

    /* 👇 新增的 UI 导航字典 */
    QS_SIG_DICTIONARY(NAV_UP_SIG, (void *)0);
    QS_SIG_DICTIONARY(NAV_DOWN_SIG, (void *)0);
    QS_SIG_DICTIONARY(NAV_ENTER_SIG, (void *)0);
    QS_SIG_DICTIONARY(NAV_BACK_SIG, (void *)0);

    /* 2. 极其关键：告诉框架，QS 追踪器启动成功！ */
        return 1U;
    /* ... */
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


