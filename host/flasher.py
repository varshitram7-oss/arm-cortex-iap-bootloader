#!/usr/bin/env python3
"""
Host-side UART flasher for the Custom IAP Bootloader (ARM Cortex-M).

Implements the host side of the command protocol documented in
bootloader.h / README.md:

    CMD   Name               Host sends                    Device replies
    0x51  BL_CMD_GET_VER     0x51                          0xA5 ACK + version byte
    0x53  BL_CMD_FLASH_ERASE 0x53                          0xA5 ACK
    0x54  BL_CMD_MEM_WRITE   0x54 | len[4 LE] | data |     0xA5 ACK per chunk
                                    crc32[4 LE]
    0x55  BL_CMD_VERIFY_CRC  0x55 | image_crc32[4 LE]      0xA5 ACK / 0x5A NACK
    0x56  BL_CMD_GO_TO_APP   0x56                          0xA5 ACK, then jumps

CRC-32 uses binascii.crc32 (IEEE 802.3, polynomial 0xEDB88320), matching
Bootloader_CalculateCRC32() in bootloader.c.

NOTE: this is the host reference implementation of the documented protocol.
The MEM_WRITE / VERIFY_CRC cases in Bootloader_HandleCommand() currently
acknowledge the command byte; extend them to consume the length / payload /
CRC framing sent here to complete the device side.

Usage:
    pip install pyserial
    python flasher.py --port COM3 --file app.bin
"""

import argparse
import binascii
import struct
import sys

try:
    import serial
except ImportError:
    sys.exit("pyserial is required: pip install pyserial")

# --- Protocol constants (must match bootloader.h) ---
BL_CMD_GET_VER     = 0x51
BL_CMD_FLASH_ERASE = 0x53
BL_CMD_MEM_WRITE   = 0x54
BL_CMD_VERIFY_CRC  = 0x55
BL_CMD_GO_TO_APP   = 0x56

BL_ACK  = 0xA5
BL_NACK = 0x5A

DEFAULT_BAUD = 115200
DEFAULT_CHUNK = 128
READ_TIMEOUT_S = 2.0


class Flasher:
    def __init__(self, port, baud=DEFAULT_BAUD):
        self.ser = serial.Serial(port, baud, timeout=READ_TIMEOUT_S)

    def _read_byte(self):
        data = self.ser.read(1)
        if not data:
            raise TimeoutError("no response from bootloader")
        return data[0]

    def _expect_ack(self, what):
        resp = self._read_byte()
        if resp == BL_NACK:
            raise RuntimeError(f"{what}: bootloader replied NACK")
        if resp != BL_ACK:
            raise RuntimeError(f"{what}: unexpected reply 0x{resp:02X}")

    def get_version(self):
        self.ser.write(bytes([BL_CMD_GET_VER]))
        self._expect_ack("GET_VER")
        return self._read_byte()

    def flash_erase(self):
        self.ser.write(bytes([BL_CMD_FLASH_ERASE]))
        self._expect_ack("FLASH_ERASE")

    def mem_write(self, image, chunk_size=DEFAULT_CHUNK):
        offset = 0
        while offset < len(image):
            chunk = image[offset:offset + chunk_size]
            crc = binascii.crc32(chunk) & 0xFFFFFFFF
            packet = (bytes([BL_CMD_MEM_WRITE])
                      + struct.pack("<I", len(chunk))
                      + chunk
                      + struct.pack("<I", crc))
            self.ser.write(packet)
            self._expect_ack(f"MEM_WRITE @0x{offset:08X}")
            offset += len(chunk)
            print(f"  wrote {offset}/{len(image)} bytes", flush=True)

    def verify_crc(self, image_crc):
        self.ser.write(bytes([BL_CMD_VERIFY_CRC]) + struct.pack("<I", image_crc))
        self._expect_ack("VERIFY_CRC")

    def go_to_app(self):
        self.ser.write(bytes([BL_CMD_GO_TO_APP]))
        self._expect_ack("GO_TO_APP")

    def close(self):
        self.ser.close()


def main():
    ap = argparse.ArgumentParser(
        description="UART host flasher for the custom ARM Cortex-M IAP bootloader")
    ap.add_argument("--port", required=True,
                    help="serial port (e.g. COM3 or /dev/ttyUSB0)")
    ap.add_argument("--baud", type=int, default=DEFAULT_BAUD)
    ap.add_argument("--file", required=True, help="firmware binary to flash")
    ap.add_argument("--chunk-size", type=int, default=DEFAULT_CHUNK)
    ap.add_argument("--skip-erase", action="store_true",
                    help="skip the flash-erase step")
    args = ap.parse_args()

    with open(args.file, "rb") as f:
        image = f.read()
    if not image:
        sys.exit("firmware image is empty")
    image_crc = binascii.crc32(image) & 0xFFFFFFFF
    print(f"Image: {args.file} ({len(image)} bytes, CRC-32 0x{image_crc:08X})")

    fl = Flasher(args.port, args.baud)
    try:
        ver = fl.get_version()
        print(f"Bootloader version: {ver >> 4}.{ver & 0x0F} (raw 0x{ver:02X})")
        if not args.skip_erase:
            print("Erasing application flash...")
            fl.flash_erase()
        print("Writing firmware...")
        fl.mem_write(image, args.chunk_size)
        print("Verifying image CRC-32...")
        fl.verify_crc(image_crc)
        print("Jumping to application...")
        fl.go_to_app()
        print("Done.")
    finally:
        fl.close()


if __name__ == "__main__":
    main()
