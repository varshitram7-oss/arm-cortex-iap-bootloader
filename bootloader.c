/**
 * @file bootloader.c
 * @brief Implementation of Custom In-Application Programming (IAP) Bootloader
 * @author Vila Ram Varshit (github.com/varshitram7-oss)
 */

#include "bootloader.h"
#include <string.h>

/* Low-level CMSIS / ARM Cortex-M hardware register definitions */
#define SCB_VTOR                (*(volatile uint32_t *)0xE000ED08UL) /* Vector Table Offset Register */

/* Simulated UART Transmit & Receive helper functions */
extern void UART_WriteByte(uint8_t byte);
extern uint8_t UART_ReadByte(void);
extern bool UART_IsDataAvailable(void);
extern bool GPIO_ReadBootPin(void);

/**
 * @brief Checks if a valid user application exists at APP_START_ADDRESS.
 * 
 * In ARM Cortex-M architecture:
 * - Address 0x00000000 (or APP_START_ADDRESS) contains the Initial Main Stack Pointer (MSP).
 * - The MSP must reside within the valid SRAM memory address range.
 */
bool Bootloader_IsAppValid(void)
{
    /* Read the initial Stack Pointer value from the first 4 bytes of Application Flash */
    uint32_t app_stack_pointer = *(volatile uint32_t *)APP_START_ADDRESS;

    /* Verify if the stack pointer is inside the valid SRAM boundary */
    if ((app_stack_pointer >= SRAM_START_ADDRESS) && (app_stack_pointer <= SRAM_END_ADDRESS))
    {
        return true; /* Valid application present */
    }
    return false;    /* Empty or corrupted application area */
}

/**
 * @brief Relocates Vector Table, sets MSP, and jumps to Application Reset Handler.
 * 
 * Step 1: Read Reset Handler function pointer from (APP_START_ADDRESS + 4 bytes).
 * Step 2: Set Main Stack Pointer (__set_MSP) to application stack base.
 * Step 3: Relocate Vector Table Offset Register (SCB->VTOR).
 * Step 4: Call application Reset Handler function pointer.
 */
void Bootloader_JumpToApplication(void)
{
    /* 1. Extract the Application Reset Handler address from Vector Table */
    uint32_t reset_handler_addr = *(volatile uint32_t *)(APP_START_ADDRESS + 4);
    void (*app_reset_handler)(void) = (void (*)(void))reset_handler_addr;

    /* 2. Set Main Stack Pointer (MSP) using inline assembly for ARM Cortex-M */
    uint32_t app_msp_value = *(volatile uint32_t *)APP_START_ADDRESS;
    __asm volatile ("MSR msp, %0" : : "r" (app_msp_value) : );

    /* 3. Relocate Vector Table to point to the User Application region */
    SCB_VTOR = APP_START_ADDRESS;

    /* 4. Branch / Jump to User Application */
    app_reset_handler();
}

/**
 * @brief 32-bit CRC (Cyclic Redundancy Check) calculation using IEEE 802.3 polynomial.
 * Standard Polynomial: 0xEDB88320 (Reversed representation of 0x04C11DB7)
 */
uint32_t Bootloader_CalculateCRC32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xEDB88320UL;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

/**
 * @brief Handles incoming command packets received over UART.
 */
static void Bootloader_HandleCommand(uint8_t cmd)
{
    switch (cmd)
    {
        case BL_CMD_GET_VER:
            UART_WriteByte(BL_ACK);
            UART_WriteByte(BL_VERSION);
            break;

        case BL_CMD_GO_TO_APP:
            if (Bootloader_IsAppValid())
            {
                UART_WriteByte(BL_ACK);
                Bootloader_JumpToApplication();
            }
            else
            {
                UART_WriteByte(BL_NACK); /* No valid app to jump to */
            }
            break;

        case BL_CMD_FLASH_ERASE:
            /* In physical hardware: Call Flash memory erase HAL/Register function */
            UART_WriteByte(BL_ACK);
            break;

        case BL_CMD_MEM_WRITE:
            /* Receive payload length, data chunk, verify CRC, write to Flash */
            UART_WriteByte(BL_ACK);
            break;

        default:
            UART_WriteByte(BL_NACK); /* Invalid command */
            break;
    }
}

/**
 * @brief Main execution entry for the Bootloader.
 */
void Bootloader_Run(void)
{
    /* Check if the user is holding the BOOT button at power-on to force bootloader mode */
    bool force_bootloader = GPIO_ReadBootPin();

    if (!force_bootloader && Bootloader_IsAppValid())
    {
        /* Normal Startup: Jump directly to User Application */
        Bootloader_JumpToApplication();
    }

    /* Otherwise, enter Bootloader Update Mode and wait for UART host commands */
    while (1)
    {
        if (UART_IsDataAvailable())
        {
            uint8_t cmd = UART_ReadByte();
            Bootloader_HandleCommand(cmd);
        }
    }
}
