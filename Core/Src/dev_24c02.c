/* dev_24c02.c */
#include "elab_device.h"
#include "elab_export.h"
#include "stm32f4xx_hal.h"  /* 引入 STM32F4 底层 HAL 库 */
#include <string.h>

/* ================================================================== */
/* 硬件参数配置 (完美契合您的需求)                                      */
/* ================================================================== */
extern I2C_HandleTypeDef hi2c1;         /* PB6(SCL), PB7(SDA) 对应的硬件 I2C1 */

#define EEPROM_ADDRESS     (0x50 << 1)  /* HAL库要求8位地址，0x50左移1位即 0xA0 */
#define EEPROM_MAX_SIZE    (256)        /* 24C02 最大容量 256 字节 */
#define I2C_TIMEOUT        (1000)       /* 超时时间 1000ms */

/* ================================================================== */
/* 1. 物理层操作函数 (VFS 接口实现)                                     */
/* ================================================================== */

static elab_err_t eeprom_enable(elab_device_t *me, bool status) {
    if (status) {
        /* 探针：上电时探测 I2C 总线上有没有这颗 0x50 的芯片！ */
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_ADDRESS, 3, I2C_TIMEOUT) != HAL_OK) {
            return ELAB_ERROR; /* 芯片没焊好或线断了，直接报错！ */
        }
    }
    return ELAB_OK;
}

static int32_t eeprom_read(elab_device_t *me, uint32_t pos, void *buffer, uint32_t size) {
    /* 防弹装甲：绝对不允许越界读取！(保护寻址范围 0x00~0xFF) */
    if (pos >= EEPROM_MAX_SIZE) return 0;
    if (pos + size > EEPROM_MAX_SIZE) {
        size = EEPROM_MAX_SIZE - pos; /* 截断多余的读取请求 */
    }

    /* 发起 I2C 硬件读取操作 (地址长度为 8-bit) */
    if (HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDRESS, pos, I2C_MEMADD_SIZE_8BIT,
                         (uint8_t*)buffer, size, I2C_TIMEOUT) == HAL_OK) {
        return size; /* 返回实际读到的字节数 */
    }
    return -1; /* 硬件级读取失败 */
}

static int32_t eeprom_write(elab_device_t *me, uint32_t pos, const void *buffer, uint32_t size) {
    /* 防弹装甲：绝对不允许越界写入！ */
    if (pos >= EEPROM_MAX_SIZE) return 0;
    if (pos + size > EEPROM_MAX_SIZE) {
        size = EEPROM_MAX_SIZE - pos;
    }

    uint8_t *pData = (uint8_t *)buffer;

    /* * 极其关键的 EEPROM 工业级写法：单字节轮询写入法！
     * 避免了复杂的 Page 写对齐（24C02 的 Page 只有 8 字节），绝对不丢数据。
     */
    for (uint32_t i = 0; i < size; i++) {
        if (HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDRESS, pos + i, I2C_MEMADD_SIZE_8BIT,
                              &pData[i], 1, I2C_TIMEOUT) != HAL_OK) {
            return i; /* 如果半路写入失败，返回已经成功写入的字节数 */
        }

        /* EEPROM 物理特性：写完一个字节必须等它的内部电荷泵烧录完毕！(通常需要 5ms) */
        while (HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_ADDRESS, 1, 10) != HAL_OK);
    }
    return size;
}

/* ================================================================== */
/* 2. 装备面向对象的面具，并塞进自动注册车厢                          */
/* ================================================================== */

/* 极其隐蔽的私有静态变量，绝不污染全局命名空间 */
static elab_device_t eeprom_dev;

static elab_device_attr_t eeprom_attr = {
    .name = "eeprom_24c02",
    .type = ELAB_DEVICE_I2C,  /* 标记为 I2C 设备 */
    .sole = true              /* 独占设备，防止多线程同时读写搞崩总线 */
};

static const elab_dev_ops_t eeprom_ops = {
    .enable = eeprom_enable,
    .read   = eeprom_read,
    .write  = eeprom_write
};

static void EEPROM_Auto_Register(void) {
    eeprom_dev.ops = &eeprom_ops;
    elab_device_register(&eeprom_dev, &eeprom_attr);
}

/* 魔法发动！给它一张 Level 2 的自动装载入场券 */
INIT_EXPORT(EEPROM_Auto_Register, 2);
