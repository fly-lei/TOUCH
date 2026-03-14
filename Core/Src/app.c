/*
 * app.c
 *
 *  Created on: Mar 14, 2026
 *      Author: 14019
 */


#include "signals.h"

/* 1. 大脑自己的私有结构体 */
typedef struct {
    QActive super;
    uint32_t press_count; /* 私有财产：按键计数器 */
} App;

static App l_app; /* 实例化一个大脑实体 */
QActive * const AO_App = &l_app.super; /* 暴露给外面的指针 */

/* 2. 状态机前置声明 */
static QState App_initial(App * const me, QEvt const * const e);
static QState App_running(App * const me, QEvt const * const e);

/* 3. 构造函数 */
void App_ctor(void) {
    QActive_ctor(&l_app.super, Q_STATE_CAST(&App_initial));
}

/* 4. 初始状态 */
static QState App_initial(App * const me, QEvt const * const e) {
    (void)e;
    me->press_count = 0; /* 开机清零 */

#ifdef Q_SPY
    QS_OBJ_DICTIONARY(&l_app);
    QS_FUN_DICTIONARY(&App_initial);
    QS_FUN_DICTIONARY(&App_running);
#endif

    return Q_TRAN(&App_running);
}

/* 5. 运行状态：拦截按键，发送带数据的事件给屏幕 */
static QState App_running(App * const me, QEvt const * const e) {
    switch (e->sig) {
        case BTN_PRESSED_SIG: {
            me->press_count++; /* 核心业务逻辑：加1 */

            /* ⚠️ 极其关键：向系统申请一个 UIUpdateEvt 大小的快递盒 */
            UIUpdateEvt *pe = Q_NEW(UIUpdateEvt, UI_UPDATE_COUNT_SIG);

            /* 把最新数据装进盒子里 */
            pe->count_val = me->press_count;

            /* 把快递盒精准投递给 GUI 部门 (0U表示无发送者追踪) */
            QACTIVE_POST(AO_Gui, &pe->super, 0U);

            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}
