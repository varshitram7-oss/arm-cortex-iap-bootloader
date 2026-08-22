/**
 * @file linker_bootloader.ld
 * @brief Linker Script for Custom Bootloader (ARM Cortex-M Architecture)
 * @author Vila Ram Varshit (github.com/varshitram7-oss)
 */

ENTRY(Reset_Handler)

/* Memory Map Definition */
MEMORY
{
  /* 32 KB Flash reserved exclusively for Bootloader starting at 0x08000000 */
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 32K
  
  /* 20 KB SRAM */
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Sections Placement */
SECTIONS
{
  /* The startup vector table goes into the very beginning of FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  /* Constant data into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  /* Initialized data sections into RAM, loaded from FLASH */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  /* Uninitialized data section into RAM */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* User stack and heap placement */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + 0x400; /* 1 KB Heap */
    . = . + 0x400; /* 1 KB Stack */
    . = ALIGN(8);
  } >RAM
}
