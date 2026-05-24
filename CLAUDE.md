# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Project

Smart-Farmer is firmware for the **Guition JC3248W535** dev board
(ESP32-S3 + 3.2" 320×480 AXS15231B QSPI display with capacitive touch). It
shows live readings from soil-moisture, SHT3x air temp+humidity, and BH1750
light sensors, drives irrigation/lighting relays, and publishes JSON
telemetry over MQTT.

Framework: **ESP-IDF v5.1+** with **LVGL v9** for UI.

## Commands

```bash
# one-time
. $IDF_PATH/export.sh

idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor   # adjust port

# Wi-Fi / broker config (no menuconfig needed)
idf.py build -DWIFI_SSID=\"myssid\" -DWIFI_PASS=\"mypass\" \
             -DMQTT_BROKER_URI=\"mqtt://192.168.1.10\"
```

LVGL and other managed components are pulled by `idf.py reconfigure` from
`main/idf_component.yml`.

## Architecture

Top-level flow lives in `main/main.c`:

1. `app_state_init` + `relays_init` — shared state & GPIO outputs
2. `display_init` — sets up QSPI bus, creates the AXS15231B esp_lcd panel,
   turns the backlight on
3. `axs15231b_touch_init` — brings up the I2C touch chip (shares the bus
   with SHT3x + BH1750)
4. `lvgl_port_start` — registers the panel + touch as LVGL display/indev,
   spawns the LVGL handler task pinned to core 1
5. `ui_dashboard_create` — builds the dashboard once; an LVGL timer
   refreshes labels from `app_state_get` every 500 ms
6. `sensors_start` — background task polls SHT3x / BH1750 / soil ADC every
   2 s and writes into `app_state_*`
7. `wifi_start_blocking` then `mqtt_start` — connects and starts a 5-s
   telemetry publisher plus a `relay/+` command subscriber

Module map:

```
main/
  board.h                 pin map (single source of truth)
  app_state.{c,h}         mutex-guarded sensor/relay snapshot
  display/
    display.{c,h}         QSPI bus + esp_lcd panel-io setup
    axs15231b_panel.{c,h} vendor panel driver (init seq, draw_bitmap)
    axs15231b_touch.{c,h} I2C touch reader
    lvgl_port.{c,h}       LVGL init, flush_cb, indev cb, handler task
  ui/ui_dashboard.{c,h}   LVGL widgets + refresh timer
  sensors/sensors.{c,h}   SHT3x + BH1750 + capacitive-soil ADC poller
  control/relays.{c,h}    GPIO relay output + state sync
  net/wifi.{c,h}          STA-mode connect helper
  net/mqtt.{c,h}          telemetry publish + relay command sub
```

## Conventions

- **Pins live in `main/board.h`.** Don't sprinkle GPIO numbers across the
  codebase — add a `BSP_*` macro and reference it.
- **Touch + SHT3x + BH1750 share `I2C_NUM_0`** on GPIO4/GPIO8. The touch
  driver owns `i2c_driver_install`; sensor modules just use
  `i2c_master_*_device` calls.
- **All access to LVGL from outside the LVGL task must be wrapped in
  `lvgl_port_lock()` / `lvgl_port_unlock()`.** The dashboard refresh runs
  inside the LVGL task via `lv_timer`, so it's already safe.
- **`app_state_*`** is the canonical place for cross-task data. UI, MQTT,
  and any future scheduler should read/write here rather than poking
  sensor or relay modules directly.
- **AXS15231B init values** in `axs15231b_panel.c` come from Guition's
  reference. If the panel turns on but shows garbled pixels or wrong
  colours, that table is the first place to look.
