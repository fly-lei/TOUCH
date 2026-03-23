/*
 * flight_recorder.c
 *
 *  Created on: Mar 23, 2026
 *      Author: 14019
 */


/* flight_recorder.c */
#include "flight_recorder.h"
#include "elab_device.h"
#include <string.h>

/* 驻留在单片机 RAM 中的黑匣子本体 */
Flight_Recorder_t g_flight_recorder = {0};

/* ================================================================== */
/* 1. 日常记录引擎 (极其轻量，0.1微秒极速执行，随便在哪调都不卡)        */
/* ================================================================== */
/* flight_recorder.c 内部 */

/* 别忘了在文件顶部引入您的 HAL 库头文件 (如果您是 F4 就用 stm32f4xx_hal.h) */
#include "stm32f4xx_hal.h"

void FR_Log_Event(uint32_t state, uint16_t sig, float val) {
    QF_CRIT_ENTRY();

    uint32_t idx = g_flight_recorder.head;

    /* 👇 就是这里！把 QF_TICK_X(0) 换成底层的 HAL_GetTick() 👇 */
    g_flight_recorder.records[idx].timestamp = HAL_GetTick();

    g_flight_recorder.records[idx].state_id = state;
    g_flight_recorder.records[idx].event_sig = sig;
    g_flight_recorder.records[idx].key_value = val;

    g_flight_recorder.head = (idx + 1) % FR_BUFFER_SIZE;

    QF_CRIT_EXIT();
}

/* main.c 内部 或 flight_recorder.c 内部 */
extern void W25Q_Read_Panic_Dump(uint32_t addr, uint8_t *data, uint32_t size);
extern void W25Q_Erase_Panic_Dump(uint32_t addr);

void Check_And_Print_Panic_Dump(void) {
    Flight_Recorder_t dump = {0};

    /* 1. 裸机强行读取，绝不看 VFS 的脸色！ */
    W25Q_Read_Panic_Dump(PANIC_SECTOR_ADDR, (uint8_t*)&dump, sizeof(Flight_Recorder_t));

    QS_BEGIN(QS_USER0, 0);
    QS_STR("\r\n[DEBUG] Blackbox Magic Read: 0x");
    QS_U32_HEX(8, dump.magic);
    QS_STR("\r\n");
    QS_END();

    /* 判断命案现场 */
    if (dump.magic == BLACKBOX_MAGIC_VALID) {

    	        QS_BEGIN(QS_USER0, 0);
    	        QS_STR("FATAL CRASH RECOVERED!\r\n");

    	        /* 1. 打印死亡模块 */
    	        QS_STR("   -> Module: ");
    	        QS_STR(dump.death_cause);

    	        /* 👇 2. 打印极其关键的死亡行号！ 👇 */
    	        QS_STR("\r\n   -> Line:   ");
    	        QS_U32(5, dump.death_location);

    	        QS_STR("\r\n--- Last 64 Steps Chronology ---\r\n");
    	        QS_END();

        for (uint16_t i = 0; i < FR_BUFFER_SIZE; i++) {
            uint16_t idx = (dump.head + i) % FR_BUFFER_SIZE;
            FR_Record_t *rec = &dump.records[idx];
            if (rec->timestamp == 0) continue;

            QS_BEGIN(QS_USER0, 0);
            QS_STR("["); QS_U32(5, rec->timestamp); QS_STR("] ");
            QS_STR("St_Addr: 0x");
            QS_U32_HEX(8, rec->state_id); /* 打印出极其专业的 0x080023ED */
           // QS_STR("St:"); QS_U16(3, rec->state_id);
            QS_STR(" Sig:"); QS_U16(3, rec->event_sig);
            QS_STR(" Val:"); QS_F32(2, rec->key_value);
            QS_STR("\r\n");
            QS_END();
        }
    } else if (dump.magic == 0xFFFFFFFF) {
        QS_BEGIN(QS_USER0, 0);
        QS_STR("[DEBUG] Sector is blank (0xFF). Ready for crash!\r\n");
        QS_END();
    } else {
        QS_BEGIN(QS_USER0, 0);
        QS_STR("[DEBUG] Garbage data read. Erase or Write corrupted!\r\n");
        QS_END();
    }

    /* 2. 裸机强行擦除！ */
    W25Q_Erase_Panic_Dump(PANIC_SECTOR_ADDR);

    /* 3. 裸机强行回读验证！ */
    uint32_t verify_magic = 0;
    W25Q_Read_Panic_Dump(PANIC_SECTOR_ADDR, (uint8_t*)&verify_magic, 4);

    if (verify_magic != 0xFFFFFFFF) {
        QS_BEGIN(QS_USER0, 0);
        QS_STR("[ERROR] Bare-Metal Erase FAILED! It read: 0x");
        QS_U32_HEX(8, verify_magic);
        QS_STR("\r\n");
        QS_END();
    } else {
        QS_BEGIN(QS_USER0, 0);
        QS_STR("[OK] Bare-Metal Flash Pre-Erased to 0xFF!\r\n");
        QS_END();
    }

    /* 恢复 RAM 记录仪 */
    memset(&g_flight_recorder, 0, sizeof(Flight_Recorder_t));
    g_flight_recorder.magic = BLACKBOX_MAGIC_VALID;
}
/* ================================================================== */
/* 2. 开机尸检报告与预擦除 (放在 main 函数 QF_run 之前调用)             */
/* ================================================================== */
//
//void Check_And_Print_Panic_Dump(void) {
//    Flight_Recorder_t dump;
//
//    /* 召唤 VFS 大管家去现场提证 */
//    elab_device_t *flash = elab_device_find("flash_w25q128");
//    if (flash == NULL) return;
//
//    /* 用 VFS 优雅地读取整个死亡扇区 */
//    elab_device_read(flash, PANIC_SECTOR_ADDR, &dump, sizeof(Flight_Recorder_t));
//
//    /* 架构师的法眼：核对魔数！如果发现命案，立刻全网通报！ */
//    if (dump.magic == BLACKBOX_MAGIC_VALID) {
//        QS_BEGIN(QS_USER0, 0);
//        QS_STR("\n=========================================\n");
//        QS_STR("🔥 FATAL CRASH RECOVERED FROM BLACKBOX 🔥\n");
//        QS_STR("=========================================\n");
//        QS_STR("Cause: "); QS_STR(dump.death_cause);
//        QS_STR(" | Loc: "); QS_U32(4, dump.death_location);
//        QS_STR("\n--- Last 64 Steps Chronology ---\n");
//        QS_END();
//
//        /* 终极解密：把环形缓冲区“拉直”按时间顺序打印！ */
//        for (uint16_t i = 0; i < FR_BUFFER_SIZE; i++) {
//            uint16_t idx = (dump.head + i) % FR_BUFFER_SIZE;
//            FR_Record_t *rec = &dump.records[idx];
//
//            if (rec->timestamp == 0) continue; /* 跳过空白记录 */
//
//            QS_BEGIN(QS_USER0, 0);
//            QS_STR("["); QS_U32(5, rec->timestamp); QS_STR("] ");
//            QS_STR("St:"); QS_U16(3, rec->state_id);
//            QS_STR(" Sig:"); QS_U16(3, rec->event_sig);
//            QS_STR(" Val:"); QS_F32(2, rec->key_value);
//            QS_END();
//        }
//        QS_BEGIN(QS_USER0, 0); QS_STR("\n--- END OF DUMP ---\n"); QS_END();
//    }
//
//    /* 👇 极其致命的闭环动作：预擦除！👇 */
//    /* 无论是否发生过命案，开机第一件事必须把这个扇区擦干净，让它时刻准备迎接下一次死亡倾倒！ */
//    uint32_t erase_addr = PANIC_SECTOR_ADDR;
//    elab_device_ioctl(flash, ELAB_IOCTL_ERASE_SECTOR, &erase_addr);
//
//    /* 清空 RAM 黑匣子，打上准备就绪的魔数，开始新一轮的陪伴 */
//    memset(&g_flight_recorder, 0, sizeof(Flight_Recorder_t));
//    g_flight_recorder.magic = BLACKBOX_MAGIC_VALID;
//}
