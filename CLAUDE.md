# CLAUDE.md

Guidance for Claude Code when working in this repo.

## Project

MicroPython firmware for the **Guition JC3248W535** — an ESP32-S3 dev board
with a 3.2" 320x480 QSPI LCD (AXS15231B controller, integrated capacitive
touch on I2C). Builds a Smart-Farmer node: LVGL dashboard + relay/pump
control + Wi-Fi/MQTT telemetry.

## Layout

```
boot.py              # Wi-Fi bring-up on power-on
main.py              # App entry: sensor loop + LVGL tick + MQTT
config.py            # User config (Wi-Fi, MQTT, pin map, calibration)
lib/
  board.py           # JC3248W535 pin definitions
  display.py         # AXS15231B QSPI driver + LVGL bind
  touch.py           # AXS15231B touch driver (I2C)
  wifi.py            # STA connect helpers
  sensors.py         # DHT22 + ADC soil/light readings
  relays.py          # GPIO relay/pump control (latching + pulse)
  mqtt.py            # umqtt.simple wrapper, command dispatch
  ui.py              # LVGL Dashboard (Sensors / Relays / Status tabs)
tools/
  flash.sh           # esptool helper
  upload.sh          # mpremote sync helper
```

## Firmware requirement

Stock MicroPython does **not** ship a QSPI LCD bus or LVGL bindings, so
`lib/display.py` and `lib/ui.py` require a custom firmware. Use the
`lvgl_micropython` build for ESP32-S3 with `lcd_bus.SPIBus(quad=True)`
support:

```
python make.py esp32 BOARD=ESP32_GENERIC_S3 BOARD_VARIANT=SPIRAM_OCT \
    DISPLAY=other --flash-size=16
```

The init sequence in `lib/display.py` is derived from the public
`Arduino_AXS15231B` reference and may need timing tweaks per panel batch.

## Commands

```bash
# Flash firmware (one-time / on updates)
tools/flash.sh /dev/ttyACM0 firmware.bin

# Push Python sources to the board
tools/upload.sh /dev/ttyACM0

# Interactive REPL
mpremote connect /dev/ttyACM0 repl
```

No host-side test suite or linter is configured yet. If/when added, list
the commands here.

## Pin map (board.py)

Display QSPI: CS=45, SCK=47, D0=21, D1=48, D2=40, D3=39, TE=38, BL=1.
Touch I2C: SDA=4, SCL=8, INT=3 (addr 0x3B). microSD shares SPI with the
display — don't access it while the LCD is being driven.

Free GPIOs for sensors/relays: 0, 10, 11, 12, 13, 14, 17, 18 (default
config uses 10–13 for relays and 17/18/14 for DHT/soil/light).

## MQTT contract

Telemetry (published every `MQTT_PUBLISH_INTERVAL_S` seconds to
`smartfarmer/telemetry`):

```json
{
  "temp_c": 24.3, "humidity_pct": 62, "soil_pct": [38],
  "light_raw": 1820, "ts_ms": 12345,
  "relays": {"Pump 1": false, "Valve A": true}
}
```

Commands subscribed on `smartfarmer/cmd/<label>`:

- `{"on": true}` / `{"on": false}` — latching set
- `{"pulse_s": 30}` — turn on for N seconds then off

`<label>` matches the strings in `config.RELAYS`.
