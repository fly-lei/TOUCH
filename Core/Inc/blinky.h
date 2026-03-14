#ifndef BLINKY_H
#define BLINKY_H

#include "qpc.h"

/* 1. 定义系统级信号 (Signals) */
enum BlinkySignals {
    TIMEOUT_SIG = Q_USER_SIG, /* 超时信号，从用户自定义信号起始值开始 */
	TOUCH_SIG,
	TIMEOUT1_SIG,
	BTN_PA1_SIG,
    //MAX_PUB_SIG               /* 信号总量上限 */
};

/* 2. 暴露 Blinky 活动对象的指针给 main 函数使用 */
extern QActive * const AO_Blinky;

/* 3. 构造函数声明 */
void Blinky_ctor(void);

#endif /* BLINKY_H */
