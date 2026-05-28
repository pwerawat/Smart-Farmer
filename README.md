# Smart-Farmer

Firmware for the **Guition JC3248W535** (ESP32-S3 + 3.2" 320×480 AXS15231B QSPI
LCD with capacitive touch). Reads soil-moisture / SHT3x / BH1750 sensors,
drives relays for irrigation or lighting, and publishes telemetry over MQTT
with a touch dashboard built in LVGL.

## Hardware

- **MCU**: ESP32-S3-WROOM-1, 16 MB flash, 8 MB octal PSRAM
- **Display**: AXS15231B, 320×480, QSPI, 16-bit colour
- **Touch**: AXS15231B over I2C
- **Sensors** (you wire these): SHT3x (I2C 0x44), BH1750 (I2C 0x23),
  capacitive soil probe on ADC1
- **Relays**: 2× GPIO outputs (active-high by default)

Default pin map is in [`main/board.h`](main/board.h). Confirm against your
exact JC3248W535 revision before wiring sensors — Guition has shipped minor
variants with different free GPIOs.

## Build

```bash
. $IDF_PATH/export.sh
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

Set credentials at build time without touching the source:

```bash
idf.py build \
  -DWIFI_SSID='"my-ssid"' \
  -DWIFI_PASS='"my-pass"' \
  -DMQTT_BROKER_URI='"mqtt://192.168.1.10:1883"'
```

## MQTT topics

Device id is the last 3 bytes of the Wi-Fi MAC.

- Telemetry (5 s, retained=false, QoS 0)
  `smart-farmer/<id>/telemetry`
  `{"t":24.3,"rh":61,"soil":42,"lux":380,"r0":0,"r1":1,"ts":12345}`
- Relay commands (QoS 1)
  `smart-farmer/<id>/relay/0` payload `on` / `off`
  `smart-farmer/<id>/relay/1` payload `on` / `off`

## Known caveats

- The AXS15231B init sequence in
  [`main/display/axs15231b_panel.c`](main/display/axs15231b_panel.c) comes
  from Guition's published reference. If you see garbled pixels or wrong
  colours on first boot, that table is the place to tweak.
- The soil-moisture conversion in `sensors.c` uses placeholder dry/wet
  raw-ADC bounds — calibrate against your actual probe and soil.
- Wi-Fi failure is non-fatal: the dashboard and local relays keep working
  offline.
