/*
 * gui.c
 *
 *  Created on: Mar 14, 2026
 *      Author: 14019
 */


/* gui.c */
#include "signals.h"
#include "ILI9341.h"
#define STR_WIDTH(chars)      ((chars) * 8) /* 计算字符串占多少个像素宽度 */
#define CENTER_X(text_width)  ((LCD_WIDTH - (text_width)) / 2)
#define PERCENT_Y(percent)    ((LCD_HEIGHT * (percent)) / 100)
/* 👇 1. 屏幕尺寸宏定义 (假设当前是 240x320 的竖屏) */
#define LCD_WIDTH  240
#define LCD_HEIGHT 320

/* 极其好用的布局宏：计算屏幕中心或按比例排版 */
#define CENTER_X(text_width)  ((LCD_WIDTH - (text_width)) / 2)
#define PERCENT_Y(percent)    ((LCD_HEIGHT * (percent)) / 100)

/* ... 结构体 Gui 定义保持不变 ... */

/* 1. 显示部门自己的私有结构体 */
typedef struct {
    QActive super;
} Gui;

static Gui l_gui;
QActive * const AO_Gui = &l_gui.super;

static QState Gui_initial(Gui * const me, QEvt const * const e);
static QState Gui_idle(Gui * const me, QEvt const * const e);

void Gui_ctor(void) {
    QActive_ctor(&l_gui.super, Q_STATE_CAST(&Gui_initial));
}


/* 2. 待机状态：专门拆快递、画屏幕 */
static QState Gui_idle(Gui * const me, QEvt const * const e) {
    switch (e->sig) {
        case UI_UPDATE_COUNT_SIG: {
            /* ⚠️ 极其关键：把普通的 QEvt 指针强转回我们的快递盒格式 */
            UIUpdateEvt const *pe = (UIUpdateEvt const *)e;

            /* 拿到数据了！以后可以在这里调用 LCD_ShowNum() 等画图函数 */
            uint32_t val_to_draw = pe->count_val;

#ifdef Q_SPY
            /* 咱们暂时没接真屏幕，用 QSPY 模拟屏幕刷新显示！ */
            QS_BEGIN_ID(QS_USER, me->super.prio)
                QS_STR("[GUI Drawing] LCD Updated -> Count: ");
                QS_U32(4, val_to_draw);
            QS_END()
#endif
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}

/* gui.c 继续 ... */

/* 提前声明状态函数 */
static QState Gui_initial(Gui * const me, QEvt const * const e);
static QState Gui_page_MainMenu(Gui * const me, QEvt const * const e);
static QState Gui_page_ScoreView(Gui * const me, QEvt const * const e);

/* 记录当前选中的菜单项 (0 或 1) */
static uint8_t menu_cursor = 0;

/* ... 构造函数保持不变 ... */

static QState Gui_initial(Gui * const me, QEvt const * const e) {
    (void)e;
    menu_cursor = 0; /* 默认选中第一项 */
#ifdef Q_SPY
    QS_OBJ_DICTIONARY(&l_gui);
    QS_FUN_DICTIONARY(&Gui_page_MainMenu);
    QS_FUN_DICTIONARY(&Gui_page_ScoreView);
#endif
    /* 开机直接进入主菜单！ */
    return Q_TRAN(&Gui_page_MainMenu);
}
/* ... */

/* ========================================================= */
/* 页面 1：主菜单页 */
/* ========================================================= */
static QState Gui_page_MainMenu(Gui * const me, QEvt const * const e) {
    switch (e->sig) {

        case Q_ENTRY_SIG: {
            /* 1. 进页面时刷黑底 */
            LCD_Clear(BLACK);

            /* 2. 画标题："MAIN MENU" (9个字符长) */
            LCD_ShowString(CENTER_X(STR_WIDTH(9)), PERCENT_Y(20), "MAIN MENU", WHITE, BLACK);

            /* 3. 画下划线装饰一下 */
            LCD_ShowString(CENTER_X(STR_WIDTH(9)), PERCENT_Y(20) + 16, "---------", GRAY, BLACK);

            /* 4. 根据光标渲染菜单项 (每行 15 个字符长，带空格对齐！) */
            if (menu_cursor == 0) {
                /* 选中：橙色醒目 */
                LCD_ShowString(CENTER_X(STR_WIDTH(15)), PERCENT_Y(40), "> 1. Score View", ORANGE, DARKBLUE);
            }else{
                /* 未选中：浅灰隐去 */
                LCD_ShowString(CENTER_X(STR_WIDTH(15)), PERCENT_Y(60), "  2. Settings  ", LIGHTGRAY, DARKBLUE);
            }
            return Q_HANDLED();
        }

        /* 菜单切换：只需改变光标，然后让系统重新触发 Q_ENTRY_SIG 局部重绘 */
        case NAV_DOWN_SIG:
        case NAV_UP_SIG: {
            menu_cursor = (menu_cursor == 0) ? 1 : 0;
            return Q_TRAN(&Gui_page_MainMenu);
        }

        /* 确认进入 */
        case NAV_ENTER_SIG: {
            if (menu_cursor == 0) {
                return Q_TRAN(&Gui_page_ScoreView);
            } else {
                /* 预留：跳入 Settings 页面 */
                return Q_HANDLED();
            }
        }
    }
    return Q_SUPER(&QHsm_top);
}
/* ========================================================= */
/* 页面 2：计分板详情页 (接收大脑发来的计数数据) */
/* ========================================================= *//* gui.c */


/* ========================================================= */
/* 页面 2：计分板详情页 */
/* ========================================================= */
static QState Gui_page_ScoreView(Gui * const me, QEvt const * const e) {
    switch (e->sig) {

        case Q_ENTRY_SIG: {
            /* 1. 进门大扫除：用您的 FSMC 函数极速刷蓝底！ */
            LCD_Clear(BLUE);

            /* 2. 画出静态框架（假设您有 LCD_ShowString 函数） */
            // LCD_ShowString(40, 32, 200, 24, 24, "--- SCORE VIEW ---");
            // LCD_ShowNum(100, 128, 0, 4, 32); /* 初始显示 0000 */
            return Q_HANDLED();
        }


        /* 在 gui.c 的 Gui_page_ScoreView 中：*/
        case UI_UPDATE_COUNT_SIG: {
            UIUpdateEvt const *pe = (UIUpdateEvt const *)e;

            /* 直接调用数字打印引擎，黄字蓝底！无需手动 LCD_Fill 掩盖！ */
            /* 因为咱们的 LCD_ShowChar 连背景色也一起涂了，自带掩盖旧数据功能！ */
            LCD_ShowNum(CENTER_X(32), PERCENT_Y(40), pe->count_val, 4, GREEN, BLUE);

            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm_top);
}
