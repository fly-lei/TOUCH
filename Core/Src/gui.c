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

/* ========================================================= */
/* 宏定义 (确保 gui.c 顶部有这些) */
/* ========================================================= */
#define STR_WIDTH(chars)      ((chars) * 8) /* 标准尺寸下，每个字符宽度固定为 8 */
#define CENTER_X(text_width)  ((LCD_WIDTH - (text_width)) / 2)
#define PERCENT_Y(percent)    ((LCD_HEIGHT * (percent)) / 100)

/* 👇 1. 屏幕尺寸宏定义 (假设当前是 240x320 的竖屏) */
#define LCD_WIDTH  240
#define LCD_HEIGHT 320


/* 在 gui.c 顶部声明 */
static uint8_t current_brightness = 100; /* 默认 100% 亮度 */

/* 极其优雅的局部刷新进度条函数 */
void Draw_ProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t percent) {
    uint16_t fill_w = (width * percent) / 100; /* 计算高亮部分的宽度 */

    /* 1. 画填充部分 (橙色) */
    if (fill_w > 0) {
        LCD_Fill(x, y, x + fill_w, y + height, ORANGE);
    }
    /* 2. 画剩余部分 (深灰色) */
    if (width > fill_w) {
        LCD_Fill(x + fill_w, y, x + width, y + height, DARKGRAY);
    }
}

/* ... 结构体 Gui 定义保持不变 ... */

/* 1. 显示部门自己的私有结构体 */
typedef struct {
    QActive super;
} Gui;

static Gui l_gui;
QActive * const AO_Gui = &l_gui.super;

static QState Gui_initial(Gui * const me, QEvt const * const e);
static QState Gui_idle(Gui * const me, QEvt const * const e);
/* 提前声明您的 4 大核心页面 */
static QState Gui_page_Home(Gui * const me, QEvt const * const e);
static QState Gui_page_MainMenu(Gui * const me, QEvt const * const e);
static QState Gui_page_SubFunc(Gui * const me, QEvt const * const e);
static QState Gui_page_SubSet(Gui * const me, QEvt const * const e);

static QState Gui_page_Brightness(Gui * const me, QEvt const * const e);

/* 光标变量，用于记录当前页面选中的是第几项 */
static uint8_t menu_cursor = 0;
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


/* ... 构造函数保持不变 ... */

static QState Gui_initial(Gui * const me, QEvt const * const e) {
    (void)e;
    menu_cursor = 0; /* 默认选中第一项 */
#ifdef Q_SPY
    QS_OBJ_DICTIONARY(&l_gui);
    QS_FUN_DICTIONARY(&Gui_page_MainMenu);
    QS_FUN_DICTIONARY(&Gui_page_ScoreView);
    QS_FUN_DICTIONARY(&Gui_page_Home);
#endif
    /* 开机直接进入主菜单！ */
    return Q_TRAN(&Gui_page_Home);
}
/* ... */

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
/* ========================================================= */
/* 页面 0：首页 (Home) */
/* ========================================================= */
static QState Gui_page_Home(Gui * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LCD_Clear(DARKBLUE);
            /* 居中显示时间："12:00:00" 是 8 个字符长 */
            LCD_ShowString(CENTER_X(STR_WIDTH(8)), PERCENT_Y(40), "12:00:00", WHITE, DARKBLUE);

            /* 底部固定栏：[首页] (占位)     [菜单] */
            LCD_ShowString(20, LCD_HEIGHT - 20, "HOME", GRAY, DARKBLUE);
            LCD_ShowString(LCD_WIDTH - 50, LCD_HEIGHT - 20, "MENU", WHITE, DARKBLUE);
            return Q_HANDLED();
        }
        case NAV_ENTER_SIG: {
            return Q_TRAN(&Gui_page_MainMenu);
        }
    }
    return Q_SUPER(&QHsm_top);
}

/* ========================================================= */
/* 页面 1：主菜单 (Main Menu) */
/* ========================================================= */
static QState Gui_page_MainMenu(Gui * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LCD_Clear(DARKBLUE);
            /* 标题："MAIN MENU" (9个字符长) */
            LCD_ShowString(CENTER_X(STR_WIDTH(9)), PERCENT_Y(15), "MAIN MENU", WHITE, DARKBLUE);

            /* 菜单项："  Function" (10个字符长，带空格对齐) */
            if (menu_cursor == 0) {
                LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(40), "> Function", ORANGE, DARKBLUE);
                LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(60), "  Setting ", LIGHTGRAY, DARKBLUE);
            } else {
                LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(40), "  Function", LIGHTGRAY, DARKBLUE);
                LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(60), "> Setting ", ORANGE, DARKBLUE);
            }

            /* 底部固定栏 */
            LCD_ShowString(20, LCD_HEIGHT - 20, "HOME", WHITE, DARKBLUE);
            LCD_ShowString(LCD_WIDTH - 50, LCD_HEIGHT - 20, "BACK", WHITE, DARKBLUE);
            return Q_HANDLED();
        }
        case NAV_DOWN_SIG:
        case NAV_UP_SIG: {
            menu_cursor = (menu_cursor == 0) ? 1 : 0;
            return Q_TRAN(&Gui_page_MainMenu);
        }
        case NAV_ENTER_SIG: {
            if (menu_cursor == 0) {
                menu_cursor = 0;
                return Q_TRAN(&Gui_page_SubFunc);
            } else {
                menu_cursor = 0;
                return Q_TRAN(&Gui_page_SubSet);
            }
        }
        case NAV_BACK_SIG:
        case NAV_HOME_SIG: {
            return Q_TRAN(&Gui_page_Home);
        }
    }
    return Q_SUPER(&QHsm_top);
}

/* ========================================================= */
/* 页面 2：二级菜单 - 功能 (Functions) */
/* ========================================================= */
static QState Gui_page_SubFunc(Gui * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LCD_Clear(DARKBLUE);
            LCD_ShowString(CENTER_X(STR_WIDTH(9)), PERCENT_Y(15), "FUNCTIONS", WHITE, DARKBLUE);

            /* 选项渲染：10个字符长 */
            LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(40), (menu_cursor == 0) ? "> Simulate" : "  Simulate", (menu_cursor == 0) ? ORANGE : LIGHTGRAY, DARKBLUE);
            LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(55), (menu_cursor == 1) ? "> Calc    " : "  Calc    ", (menu_cursor == 1) ? ORANGE : LIGHTGRAY, DARKBLUE);
            LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(70), (menu_cursor == 2) ? "> IoT Net " : "  IoT Net ", (menu_cursor == 2) ? ORANGE : LIGHTGRAY, DARKBLUE);

            LCD_ShowString(20, LCD_HEIGHT - 20, "HOME", WHITE, DARKBLUE);
            LCD_ShowString(LCD_WIDTH - 50, LCD_HEIGHT - 20, "BACK", WHITE, DARKBLUE);
            return Q_HANDLED();
        }
        case NAV_DOWN_SIG: {
            menu_cursor = (menu_cursor + 1) % 3;
            return Q_TRAN(&Gui_page_SubFunc);
        }
        case NAV_UP_SIG: {
            menu_cursor = (menu_cursor == 0) ? 2 : menu_cursor - 1;
            return Q_TRAN(&Gui_page_SubFunc);
        }
        case NAV_BACK_SIG: {
            menu_cursor = 0; /* 退回主菜单时，光标复位到第一项 */
            return Q_TRAN(&Gui_page_MainMenu);
        }
        case NAV_HOME_SIG: {
            return Q_TRAN(&Gui_page_Home);
        }
    }
    return Q_SUPER(&QHsm_top);
}

/* ========================================================= */
/* 页面 3：二级菜单 - 设置 (Settings) */
/* ========================================================= */
static QState Gui_page_SubSet(Gui * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LCD_Clear(DARKBLUE);
            LCD_ShowString(CENTER_X(STR_WIDTH(8)), PERCENT_Y(15), "SETTINGS", WHITE, DARKBLUE);

            LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(40), (menu_cursor == 0) ? "> Time Set" : "  Time Set", (menu_cursor == 0) ? ORANGE : LIGHTGRAY, DARKBLUE);
            LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(55), (menu_cursor == 1) ? "> Password" : "  Password", (menu_cursor == 1) ? ORANGE : LIGHTGRAY, DARKBLUE);
           // LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(70), (menu_cursor == 2) ? "> Params  " : "  Params  ", (menu_cursor == 2) ? ORANGE : LIGHTGRAY, DARKBLUE);
            LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(70), (menu_cursor == 2) ? "> Brightness" : "  Brightness", (menu_cursor == 2) ? ORANGE : LIGHTGRAY, DARKBLUE);

                       // LCD_ShowString(20, LCD_HEIGHT - 20, "HOME", WHITE, DARKBLUE);

            LCD_ShowString(20, LCD_HEIGHT - 20, "HOME", WHITE, DARKBLUE);
            LCD_ShowString(LCD_WIDTH - 50, LCD_HEIGHT - 20, "BACK", WHITE, DARKBLUE);
            return Q_HANDLED();
        }
        case NAV_DOWN_SIG: {
            menu_cursor = (menu_cursor + 1) % 3;
            return Q_TRAN(&Gui_page_SubSet);
        }
        case NAV_ENTER_SIG: {
                    if (menu_cursor == 2) {
                        return Q_TRAN(&Gui_page_Brightness); /* 进入亮度调节页面！ */
                    }
                    return Q_HANDLED();
                }
        case NAV_UP_SIG: {
            menu_cursor = (menu_cursor == 0) ? 2 : menu_cursor - 1;
            return Q_TRAN(&Gui_page_SubSet);
        }
        case NAV_BACK_SIG: {
            menu_cursor = 1; /* 退回主菜单时，光标指回设置项 */
            return Q_TRAN(&Gui_page_MainMenu);
        }
        case NAV_HOME_SIG: {
            return Q_TRAN(&Gui_page_Home);
        }
    }
    return Q_SUPER(&QHsm_top);
}
/* ========================================================= */
/* 页面 4：亮度调节页 (Brightness Control)                     */
/* ========================================================= */
static QState Gui_page_Brightness(Gui * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            LCD_Clear(DARKBLUE);
            LCD_ShowString(CENTER_X(STR_WIDTH(10)), PERCENT_Y(20), "BRIGHTNESS", WHITE, DARKBLUE);

            /* 提示用户如何操作 */
            LCD_ShowString(CENTER_X(STR_WIDTH(13)), PERCENT_Y(35), "Swipe UP/DOWN", GRAY, DARKBLUE);

            /* 初次渲染数字和进度条 */
            LCD_ShowNum(CENTER_X(STR_WIDTH(3)), PERCENT_Y(50), current_brightness, 3, YELLOW, DARKBLUE);
            Draw_ProgressBar(CENTER_X(160), PERCENT_Y(65), 160, 16, current_brightness);

            LCD_ShowString(LCD_WIDTH - 50, LCD_HEIGHT - 20, "BACK", WHITE, DARKBLUE);
            return Q_HANDLED();
        }

        /* 👇 核心逻辑：手势上下滑动控制亮度！ 👇 */
        case NAV_UP_SIG: {
            if (current_brightness < 100) current_brightness += 10; /* 每次增加 10% */
            goto UPDATE_UI;
        }
        case NAV_DOWN_SIG: {
            if (current_brightness > 10) current_brightness -= 10;  /* 每次减少 10% (保留最低10%防黑屏) */
            goto UPDATE_UI;
        }

UPDATE_UI:
            /* 1. 物理层：瞬间改变真实背光亮度！ */
            LCD_SetBrightness(current_brightness);

            /* 2. 视觉层：局部极其丝滑地刷新数字和进度条 (绝对不闪屏) */
            LCD_ShowNum(CENTER_X(STR_WIDTH(3)), PERCENT_Y(50), current_brightness, 3, YELLOW, DARKBLUE);
            Draw_ProgressBar(CENTER_X(160), PERCENT_Y(65), 160, 16, current_brightness);
            return Q_HANDLED();

        case NAV_BACK_SIG: {
            return Q_TRAN(&Gui_page_SubSet);
        }
    }
    return Q_SUPER(&QHsm_top);
}
