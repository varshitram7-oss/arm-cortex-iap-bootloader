# Firmware Update Sequence

ASCII sequence diagram of a full update session between the host
(`host/flasher.py`) and the bootloader over UART @ 115200 8N1.
Command codes match `bootloader.h`.

```text
Host (flasher.py)                         Target (bootloader @ 0x08000000)
      |                                               |
      |--- 0x51 BL_CMD_GET_VER --------------------->|
      |<-- 0xA5 ACK, 0x10 version byte --------------|
      |                                               |
      |--- 0x53 BL_CMD_FLASH_ERASE ----------------->|
      |<-- 0xA5 ACK ---------------------------------|
      |                                               |
      |--- 0x54 BL_CMD_MEM_WRITE ------------------->|
      |    + len (4B LE) + chunk + CRC-32 (4B LE)     |
      |<-- 0xA5 ACK (per chunk) ---------------------|
      |              ... repeat per chunk ...         |
      |                                               |
      |--- 0x55 BL_CMD_VERIFY_CRC ------------------>|
      |    + image CRC-32 (4B LE)                     |
      |<-- 0xA5 ACK (match) / 0x5A NACK (mismatch) ---|
      |                                               |
      |--- 0x56 BL_CMD_GO_TO_APP ------------------->|
      |<-- 0xA5 ACK ---------------------------------|
      |         (validates MSP, sets MSP, relocates   |
      |          SCB->VTOR, jumps to app reset        |
      |          handler @ 0x08008000)                |
```

## Failure handling

- Any `0x5A` (NACK) aborts the session — the host must not proceed to
  `GO_TO_APP`. The previous application (if any) is left untouched until
  `FLASH_ERASE` succeeds.
- A UART timeout while waiting for ACK/NACK is treated the same as a NACK.
- `GO_TO_APP` re-validates the application (MSP inside the SRAM range)
  before jumping; if invalid, the bootloader stays in update mode.
