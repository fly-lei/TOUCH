/*
 * eLab Project (Modified for Pure Bare-Metal & QP/C Framework)
 * 删除了所有 RTOS、Linux 信号、以及阻塞型 Poll 轮询机制。
 */

/* include ------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "elab_export.h"
#include "qpc.h" /* 引入 QP 框架，复用其断言 */

#ifdef __cplusplus
extern "C" {
#endif

/* 极其暴力的断言劫持 (无需包含外部 elab_assert.h) */
#ifndef assert
#define assert(test_) Q_ASSERT(test_)
#endif

/* private function prototype ----------------------------------------------- */
static void module_null_init(void);
static void _init_func_execute(int8_t level);
static void _get_init_export_table(void);

/* private variables -------------------------------------------------------- */
/* 放置一个极小的锚点，用于在内存中定位自动注册段的起始和结束 */
INIT_EXPORT(module_null_init, 0);

static elab_export_t *export_init_table = NULL;
static uint32_t count_export_init = 0;
static int8_t export_level_max = INT8_MIN;







/* 直接向 GCC 链接器要这两个神圣的隐藏指针！ */
extern const elab_export_t __start_elab_export;
extern const elab_export_t __stop_elab_export;

void elab_auto_init(void)
{
    const elab_export_t *ptr;
    int8_t max_level = 0;

    /* 1. 扫描最高优先级 */
    for (ptr = &__start_elab_export; ptr < &__stop_elab_export; ptr++) {
        if (ptr->magic_head == EXPORT_ID_INIT && ptr->level > max_level) {
            max_level = ptr->level;
        }
    }

    /* 2. 按优先级依次执行初始化函数 */
    for (int8_t lvl = 0; lvl <= max_level; lvl++) {
        for (ptr = &__start_elab_export; ptr < &__stop_elab_export; ptr++) {

            if (ptr->magic_head == EXPORT_ID_INIT && ptr->level == lvl) {
                if (ptr->func != NULL) {
                    /* 打印一下，享受看着模块自动唤醒的快感！ */
                    // printf("Auto Loading: %s (Level %d)\n", ptr->name, ptr->level);

                    /* 极其爽快地执行函数！ */
                    ((void (*)(void))ptr->func)();
                }
            }

        }
    }
}





/* public function ---------------------------------------------------------- */
//
///**
// * @brief 极其优雅的开放接口：eLab 一键自动装载！
// * 替代了原版带有死循环的 elab_run()。
// * 请在 main() 函数中，QF_init() 之后调用此函数。
// */
////
/////* 👇 直接向 GCC 链接器要这两个隐藏指针！(注意：不需要在任何地方定义它们，链接器自己会无中生有) 👇 */
////extern const elab_export_t __start_elab_export;
////extern const elab_export_t __stop_elab_export;
//void elab_auto_init(void)
//{
//    /* 1. 在内存里扫描并锁定所有的设备/模块简历 */
//    _get_init_export_table();
//
//    /* 2. 严格按照优先级(Level 0 到 5)，依次调用它们的初始化函数！ */
//    for (int8_t level = 0; level <= export_level_max; level++)
//    {
//        _init_func_execute(level);
//    }
//}

/* private function --------------------------------------------------------- */

/**
 * @brief 核心魔法：通过 "Magic Number" 向上向下寻找内存中的注册表边界
 */
static void _get_init_export_table(void)
{
    elab_export_t *func_block = (elab_export_t *)&init_module_null_init;
    uintptr_t address_last; /* 使用标准 C 的 uintptr_t 替代 elab_pointer_t */
    
    /* 向上寻找表头 */
    while (1)
    {
        address_last = ((uintptr_t)func_block - sizeof(elab_export_t));
        elab_export_t *table = (elab_export_t *)address_last;
        if (table->magic_head != EXPORT_ID_INIT ||
            table->magic_tail != EXPORT_ID_INIT)
        {
            break;
        }
        func_block = table;
    }
    export_init_table = func_block;

    /* 向下寻找表尾，并统计最大 Level */
    uint32_t i = 0;
    while (1)
    {
        if (export_init_table[i].magic_head == EXPORT_ID_INIT &&
            export_init_table[i].magic_tail == EXPORT_ID_INIT)
        {
            if (export_init_table[i].level > export_level_max)
            {
                export_level_max = export_init_table[i].level;
            }
            i ++;
        }
        else
        {
            break;
        }
    }
    count_export_init = i;
}

/**
 * @brief 执行特定 Level 的初始化函数
 */
static void _init_func_execute(int8_t level)
{
    for (uint32_t i = 0; i < count_export_init; i ++)
    {
        if (export_init_table[i].level == level)
        {
            if (!export_init_table[i].exit)
            {
                /* 调用各个模块偷偷注册的初始化函数 (比如 LED_Auto_Register) */
                ((void (*)(void))export_init_table[i].func)();
            }
        }
    }
}

/**
 * @brief eLab 的空函数锚点
 */
static void module_null_init(void)
{
    /* NULL */
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
