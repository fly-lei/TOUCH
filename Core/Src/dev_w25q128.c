/*
 * dev_w25q128.c
 *
 *  Created on: Mar 22, 2026
 *      Author: 14019
 */


/* dev_w25q128.c */
#include "elab_device.h"
#include "elab_export.h"
#include "stm32f4xx_hal.h"
#include "qpc.h"
/* ================================================================== */
/* 硬件参数与指令集配置                                                 */
/* ================================================================== */
extern SPI_HandleTypeDef hspi1; /* PA5(SCK), PA6(MISO), PA7(MOSI) 对应的 SPI1 */

#define W25Q_CS_PORT       GPIOC
#define W25Q_CS_PIN        GPIO_PIN_13

#define W25Q_CS_LOW()      HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, GPIO_PIN_RESET)
#define W25Q_CS_HIGH()     HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, GPIO_PIN_SET)

/* W25Q128 核心指令集 */
#define W25Q_CMD_JEDEC_ID  0x9F  /* 读取芯片 ID */
#define W25Q_CMD_WRITE_EN  0x06  /* 写使能 */
#define W25Q_CMD_READ_STAT 0x05  /* 读状态寄存器1 */
#define W25Q_CMD_READ_DATA 0x03  /* 读数据 */
#define W25Q_CMD_PAGE_PROG 0x02  /* 页编程 (最大256字节) */
#define W25Q_CMD_SECTOR_ERASE 0x20 /* 扇区擦除 (4KB) */

#define W25Q_MAX_SIZE      (16 * 1024 * 1024) /* 16MB */
#define W25Q_PAGE_SIZE     256

/* ================================================================== */
/* 内部微操函数 (底层驱动的灵魂)                                        */
/* ================================================================== */

/* 极其关键的等待函数：无论擦除还是写入，都必须等芯片内部忙完！ */
static void W25Q_WaitBusy(void) {
    uint8_t cmd = W25Q_CMD_READ_STAT;
    uint8_t status = 0;
    W25Q_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    do {
        HAL_SPI_Receive(&hspi1, &status, 1, 100);
    } while ((status & 0x01) == 0x01); /* BUSY 位为 1 时死等 */
    W25Q_CS_HIGH();
}

/* 开启写使能的护盾 */
static void W25Q_WriteEnable(void) {
    uint8_t cmd = W25Q_CMD_WRITE_EN;
    W25Q_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    W25Q_CS_HIGH();
}

/* ================================================================== */
/* 1. 物理层操作函数 (VFS 接口实现)                                     */
/* ================================================================== */
/* dev_w25q128.c 内部 */

static elab_err_t w25q_enable(elab_device_t *me, bool status) {
    if (status) {
        uint8_t cmd = 0x9F; /* W25Q_CMD_JEDEC_ID */
        uint8_t id[3] = {0};

        W25Q_CS_LOW();
        HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
        HAL_SPI_Receive(&hspi1, id, 3, 100);
        W25Q_CS_HIGH();

        /* 👇 架构师的显微镜：强行打印读到的 3 个字节 👇 */
        uint32_t full_id = (id[0] << 16) | (id[1] << 8) | id[2];

        QS_BEGIN(QS_USER0, 0);
        QS_STR("SPI Read ID: 0x");
        QS_U32_HEX(6, full_id); /* 以 16 进制打印这 3 个字节 */
        QS_END();

        /* 暂时放宽安检，方便我们看日志！如果以后确认了 ID，再改回来 */
        if (id[0] == 0x00 || id[0] == 0xFF) {
            return ELAB_ERROR; /* 读到全 0 或全 1，说明 SPI 物理层彻底瘫痪 */
        }

        /* 如果厂家是 Winbond(0xEF) 或 GigaDevice(0xC8)，且容量是 128Mbit(0x4018)，都放行！ */
        if ((id[0] == 0xEF || id[0] == 0xC8) && id[1] == 0x40 && id[2] == 0x18) {
            return ELAB_OK;
        } else {
            return ELAB_ERROR; /* 容量或厂家不对 */
        }
    }
    return ELAB_OK;
}
static int32_t w25q_read(elab_device_t *me, uint32_t pos, void *buffer, uint32_t size) {
    if (pos >= W25Q_MAX_SIZE) return 0;
    if (pos + size > W25Q_MAX_SIZE) size = W25Q_MAX_SIZE - pos;

    uint8_t cmd[4];
    cmd[0] = W25Q_CMD_READ_DATA;
    cmd[1] = (uint8_t)((pos >> 16) & 0xFF); /* 24位地址高字节 */
    cmd[2] = (uint8_t)((pos >> 8) & 0xFF);  /* 24位地址中字节 */
    cmd[3] = (uint8_t)(pos & 0xFF);         /* 24位地址低字节 */

    W25Q_WaitBusy(); /* 确保芯片空闲 */
    W25Q_CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
    HAL_SPI_Receive(&hspi1, (uint8_t*)buffer, size, 1000); /* 极速 DMA/轮询读取 */
    W25Q_CS_HIGH();

    return size;
}

static int32_t w25q_write(elab_device_t *me, uint32_t pos, const void *buffer, uint32_t size) {
    if (pos >= W25Q_MAX_SIZE) return 0;
    if (pos + size > W25Q_MAX_SIZE) size = W25Q_MAX_SIZE - pos;

    uint8_t *pData = (uint8_t *)buffer;
    uint32_t bytes_written = 0;

    /* 极其强悍的页边界切割算法：防止跨页写入导致数据从页头覆写！ */
    uint32_t page_remain = W25Q_PAGE_SIZE - (pos % W25Q_PAGE_SIZE);
    if (size <= page_remain) page_remain = size;

    while (size > 0) {
        W25Q_WriteEnable(); /* 每次编程前必须解开写保护 */

        uint8_t cmd[4];
        cmd[0] = W25Q_CMD_PAGE_PROG;
        cmd[1] = (uint8_t)((pos >> 16) & 0xFF);
        cmd[2] = (uint8_t)((pos >> 8) & 0xFF);
        cmd[3] = (uint8_t)(pos & 0xFF);

        W25Q_CS_LOW();
        HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
        HAL_SPI_Transmit(&hspi1, pData, page_remain, 1000);
        W25Q_CS_HIGH();

        W25Q_WaitBusy(); /* 等待这一页烧录完毕 */

        /* 移动指针，准备下一页 */
        pos += page_remain;
        pData += page_remain;
        bytes_written += page_remain;
        size -= page_remain;

        /* 如果还有数据，剩下的肯定能整页整页写 */
        page_remain = (size > W25Q_PAGE_SIZE) ? W25Q_PAGE_SIZE : size;
    }
    return bytes_written;
}

/* dev_w25q128.c 内部 */
static void W25Q_Erase_Sector(uint32_t SectorAddr) {
    W25Q_WriteEnable();
    uint8_t cmd[4];
    cmd[0] = 0x20; /* W25Q_CMD_SECTOR_ERASE */
    cmd[1] = (uint8_t)((SectorAddr >> 16) & 0xFF);
    cmd[2] = (uint8_t)((SectorAddr >> 8) & 0xFF);
    cmd[3] = (uint8_t)(SectorAddr & 0xFF);

    W25Q_CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
    W25Q_CS_HIGH();

    W25Q_WaitBusy(); /* 死等芯片内部擦除完毕 */
}

/* 👇 新增：对接 VFS 的控制枢纽 👇 */
static int32_t w25q_ctrl(elab_device_t *me, int32_t cmd, void *arg) {
    switch (cmd) {

        case ELAB_IOCTL_ERASE_SECTOR: {
            if (arg == NULL) return -1; /* 防弹：必须传地址参数！ */
            uint32_t addr = *((uint32_t *)arg); /* 把 void* 解析成我们需要的 32位地址 */
            W25Q_Erase_Sector(addr);
            return 0; /* 成功 */
        }

        case ELAB_IOCTL_ERASE_CHIP: {
            /* 未来您可以轻松扩展全片擦除命令 */
            return 0;
        }

        default:
            return -1; /* 不认识的命令，直接拒绝 */
    }
}




/* ================================================================== */
/* 2. 装备面向对象的面具，并塞进自动注册车厢                          */
/* ================================================================== */





static elab_device_t w25q_dev;

static elab_device_attr_t w25q_attr = {
    .name = "flash_w25q128",
    .type = ELAB_DEVICE_SPI,  /* 标记为 SPI 设备 */
    .sole = true              /* 独占设备 */
};

static const elab_dev_ops_t w25q_ops = {
    .enable = w25q_enable,
    .read   = w25q_read,
    .write  = w25q_write,
	.ctrl   = w25q_ctrl
};

static void W25Q128_Auto_Register(void) {
    /* 强行把 CS 引脚拉高，防止刚上电时误触发！ */
    W25Q_CS_HIGH();
    w25q_dev.ops = &w25q_ops;
    elab_device_register(&w25q_dev, &w25q_attr);
}

/* 魔法发动！给它一张 Level 3 的自动装载入场券 */
INIT_EXPORT(W25Q128_Auto_Register, 3);
