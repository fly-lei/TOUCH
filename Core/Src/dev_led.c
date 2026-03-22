/*
 * dev_led.c
 *
 *  Created on: Mar 22, 2026
 *      Author: 14019
 */


/* dev_led.c */
#include "elab_device.h"
#include "elab_export.h"
#include "qpc.h"
#include "main.h"  /* 引入 HAL 库的 GPIO 操作 */
#define IOX GPIOB
#define PINX GPIO_PIN_2

static elab_device_t led_dev;

/* ========================================================= */
/* 1. 实现标准接口：Enable (设备的初始化或开关)                 */
/* ========================================================= */
static elab_err_t led_enable(elab_device_t *me, bool status) {
    /* status 为 true 表示打开设备，false 表示关闭 */
    /* 咱们这里简单点，打开就亮灯，关闭就灭灯 */
//	__HAL_RCC_GPIOB_CLK_ENABLE();
	QS_BEGIN(QS_USER0, 0);
	    QS_STR("==> Inside led_enable!");
	    QS_END();
    if (status) {
       HAL_GPIO_WritePin(IOX, PINX, GPIO_PIN_RESET); /* 假设低电平点亮 */
    } else {
        HAL_GPIO_WritePin(IOX, PINX, GPIO_PIN_SET);   /* 高电平熄灭 */
    }
    return ELAB_OK;
}

/* ========================================================= */
/* 2. 实现标准接口：Write (给设备下发命令)                     */
/* ========================================================= */
static int32_t led_write(elab_device_t *me, uint32_t pos, const void *buffer, uint32_t size) {
    if (size != sizeof(uint8_t)) return ELAB_EINVAL; /* 拦截长度不对的乱操作 */

    uint8_t cmd = *(uint8_t*)buffer;

    /* 定义我们自己的私有协议：0=灭，1=亮，2=翻转 */
    if (cmd == 0) {
        HAL_GPIO_WritePin(IOX, PINX, GPIO_PIN_SET);
    } else if (cmd == 1) {
        HAL_GPIO_WritePin(IOX, PINX, GPIO_PIN_RESET);
    } else if (cmd == 2) {
        HAL_GPIO_TogglePin(IOX, PINX);
    }

    return size; /* 返回成功写入的字节数 */
}

/* ========================================================= */
/* 3. 实现标准接口：Read (读取设备当前状态)                    */
/* ========================================================= */
static int32_t led_read(elab_device_t *me, uint32_t pos, void *buffer, uint32_t size) {
    if (size != sizeof(uint8_t)) return ELAB_EINVAL;

    /* 读取真实物理引脚的电平状态返回给上层 */
    uint8_t current_state = HAL_GPIO_ReadPin(IOX, PINX);
    *(uint8_t*)buffer = current_state;

    return size;
}
/* dev_led.c */

/* 👇 极其关键：必须放在所有函数的外边！让它们分配在全局 .bss / .data 段！ 👇 */
static elab_device_t led_dev;

static elab_device_attr_t led_attr = {
    .name = "led_status",
    .type = ELAB_DEVICE_PIN,
    .sole = false
};

static const elab_dev_ops_t led_ops = {
    .enable = led_enable,
    .read   = led_read,
    .write  = led_write
};

/* 初始化函数现在变得极其清爽 */
void LED_Device_Init(void) {
    led_dev.ops = &led_ops;
    elab_device_register(&led_dev, &led_attr);
}
/* ========================================================= */
/* 5. 极其神圣的自动注册魔法！                               */
/* ========================================================= */
static void LED_Auto_Register(void) {
    led_dev.ops = &led_ops;
    elab_device_register(&led_dev, &led_attr);
}

/* 分配到 Device 层 (第 3 级)，开机自动执行！ */
INIT_EXPORT(LED_Auto_Register, EXPORT_DEVICE);
