#ifndef SIGNALS_H
#define SIGNALS_H

#include "qpc.h"

/* 1. 定义全系统的信号 ID (暗号) */
/* signals.h */
/* ... 之前的代码 ... */

enum AppSignals {
    BTN_PRESSED_SIG = Q_USER_SIG,
    TOUCH_DETECTED_SIG,
    UI_UPDATE_COUNT_SIG,

    /* 👇 新增：UI 专用的导航信号 (可以由按键或触摸屏滑动触发) */
    NAV_UP_SIG,
    NAV_DOWN_SIG,
    NAV_ENTER_SIG,
    NAV_BACK_SIG,

    MAX_PUB_SIG,
};
/* ... 结构体保持不变 ... */

/* 2. 定义带有数据的自定义事件 (快递盒) */
typedef struct {
    QEvt super;         /* 必须放在第一项！继承标准事件结构 */
    uint32_t count_val; /* 里面装的是我们要传给屏幕的最新计数值 */
} UIUpdateEvt;

/* 3. 声明两个活动对象的全局指针，方便互相发消息 */
extern QActive * const AO_App;
extern QActive * const AO_Gui;

#endif /* SIGNALS_H */
