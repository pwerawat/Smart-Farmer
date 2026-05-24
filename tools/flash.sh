#!/usr/bin/env bash
# Flash an lvgl_micropython firmware image to the JC3248W535.
# Usage: tools/flash.sh /dev/ttyACM0 firmware.bin
set -euo pipefail

PORT="${1:-/dev/ttyACM0}"
FIRMWARE="${2:-firmware.bin}"

if ! command -v esptool.py >/dev/null; then
    echo "esptool.py not on PATH: pip install esptool" >&2
    exit 1
fi

esptool.py --chip esp32s3 --port "$PORT" --baud 921600 erase_flash
esptool.py --chip esp32s3 --port "$PORT" --baud 921600 \
    write_flash -z --flash_mode dio --flash_size 16MB 0x0 "$FIRMWARE"
