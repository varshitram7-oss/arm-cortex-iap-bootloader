/**
 * @file bootloader.h
 * @brief Custom In-Application Programming (IAP) Bootloader for ARM Cortex-M
 * @author Vila Ram Varshit (github.com/varshitram7-oss)
 */

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/*                      MEMORY CONFIGURATION & ADDRESSES                      */
/* ========================================================================== */

#define FLASH_BASE_ADDR         (0x08000000UL)
#define BOOTLOADER_SIZE         (0x00008000UL) /* 32 KB allocated for Bootloader */
#define APP_START_ADDRESS       (FLASH_BASE_ADDR + BOOTLOADER_SIZE) /* 0x08008000 */
#define SRAM_START_ADDRESS      (0x20000000UL)
#define SRAM_SIZE               (20 * 1024UL)  /* 20 KB SRAM (e.g. STM32F103) */
#define SRAM_END_ADDRESS        (SRAM_START_ADDRESS + SRAM_SIZE)

/* ========================================================================== */
/*                      BOOTLOADER COMMAND PROTOCOL                           */
/* ========================================================================== */

#define BL_CMD_GET_VER          (0x51) /* Get Bootloader Version */
#define BL_CMD_GET_HELP         (0x52) /* Get Supported Commands */
#define BL_CMD_FLASH_ERASE      (0x53) /* Erase Application Flash Area */
#define BL_CMD_MEM_WRITE        (0x54) /* Write Firmware Chunk to Flash */
#define BL_CMD_VERIFY_CRC       (0x55) /* Verify CRC-32 Checksum */
#define BL_CMD_GO_TO_APP        (0x56) /* Jump to User Application */

#define BL_ACK                  (0xA5)
#define BL_NACK                 (0x5A)

#define BL_VERSION              (0x10) /* Version 1.0 */

/* ========================================================================== */
/*                      FUNCTION PROTOTYPES                                   */
/* ========================================================================== */

/**
 * @brief Initializes peripherals required for bootloader (UART, GPIO, Timers).
 */
void Bootloader_Init(void);

/**
 * @brief Main polling loop listening for host firmware flashing commands.
 */
void Bootloader_Run(void);

/**
 * @brief Checks if valid application firmware exists at APP_START_ADDRESS.
 * @return true if valid MSP is found in SRAM range, false otherwise.
 */
bool Bootloader_IsAppValid(void);

/**
 * @brief Relocates Vector Table, updates MSP, and branches to Application Reset Handler.
 */
void Bootloader_JumpToApplication(void);

/**
 * @brief Calculates 32-bit CRC checksum over a data buffer for integrity validation.
 */
uint32_t Bootloader_CalculateCRC32(const uint8_t *data, uint32_t length);

#endif /* BOOTLOADER_H */
