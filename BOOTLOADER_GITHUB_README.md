Custom In-Application Programming (IAP) Bootloader for ARM Cortex-M
Language: CTarget: ARM Cortex-M 
License: MIT

A lightweight, bare-metal In-Application Programming (IAP) Bootloader written in C for ARM Cortex-M (STM32) microcontrollers. Enables in-field firmware updates over UART with CRC-32 integrity validation, flash memory protection, and seamless execution handover to user applications.

📌 Features
Partitioned Flash Memory Architecture: Dedicated 32 KB Bootloader area (0x08000000) and 96 KB User Application area (0x08008000).
UART Command Engine: Packet-based protocol supporting commands for version checks, flash erasing, binary flashing, and application branching.
Data Integrity (CRC-32): Validates received binary firmware payload using hardware/software 32-bit CRC calculation to prevent system bricking.
Safe Application Jump:
Validates Main Stack Pointer (MSP) boundaries in SRAM.
Sets the CPU MSP register via inline assembly.
Relocates the Vector Table Offset Register (SCB->VTOR).
Calls the application's Reset Handler function pointer.
Dual Boot Mode: Checks for hardware button hold at power-on to force firmware update mode or jump directly to the user application.
🗺️ Memory Map Layout
text

+-------------------------------------------------------------+ 0x08000000
|                    BOOTLOADER REGION                        | (32 KB)
|  - Vector Table (.isr_vector)                               |
|  - Bootloader Code (.text) & Const Data (.rodata)           |
|  - UART Protocol Engine & CRC-32 Validator                  |
+-------------------------------------------------------------+ 0x08008000
|                  USER APPLICATION REGION                    | (96 KB)
|  - User App Vector Table                                    |
|  - User Firmware Logic                                      |
+-------------------------------------------------------------+ 0x08020000
🔄 Boot Sequence Flowchart
text

               +-----------------------------+
               |      MCU Power-On / Reset   |
               +--------------+--------------+
                              |
                              v
               +-----------------------------+
               |  Bootloader Starts (0x0800) |
               +--------------+--------------+
                              |
               +--------------v--------------+
               |  Is Bootloader Pin Active?  |
               +------+---------------+------+
                 YES  |               |  NO
                      v               |
         +-----------------------+    |
         | Wait for UART Command |    |
         | (Flash Erase/Write)   |    |
         +------------+----------+    |
                      |               |
                      +-------+-------+
                              |
                              v
               +-----------------------------+
               | Is Valid App in Flash?      |
               | (Check MSP value in SRAM)   |
               +--------------+--------------+
                              | YES
                              v
               +-----------------------------+
               | 1. Update MSP Register      |
               | 2. Relocate VTOR Register   |
               | 3. Jump to App ResetHandler |
               +-----------------------------+
🛠️ UART Command Protocol
Command Code	Command Name	Description
0x51	BL_CMD_GET_VER	Returns the current bootloader version byte
0x53	BL_CMD_FLASH_ERASE	Erases the allocated user application Flash sector
0x54	BL_CMD_MEM_WRITE	Flashes incoming binary chunk with CRC validation
0x55	BL_CMD_VERIFY_CRC	Verifies CRC-32 of flashed application binary
0x56	BL_CMD_GO_TO_APP	Relocates vector table and jumps to user application
🚀 How to Build & Run
Prerequisites
arm-none-eabi-gcc toolchain
make utility or STM32CubeIDE
QEMU (for ARM Cortex-M emulation) or physical STM32 Nucleo board
Compilation
bash

# Compile Bootloader binary
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -T linker_bootloader.ld -o bootloader.elf bootloader.c
# Generate raw binary for flashing
arm-none-eabi-objcopy -O binary bootloader.elf bootloader.bin
👤 Author
Vila Ram Varshit

LinkedIn: linkedin.com/in/ram-varshit-ece
GitHub: github.com/varshitram7-oss
Email: 
ramvarshit18@gmail.com
