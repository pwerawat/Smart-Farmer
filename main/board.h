#pragma once

// Pin map for Guition JC3248W535 (ESP32-S3, 3.2" 320x480 QSPI, AXS15231B LCD+touch).
// Verify against your board variant — Guition has shipped minor revisions with
// different relay/audio pins. Display + touch pins below match the C/N variants.

// ---- Display: AXS15231B over QSPI ----
#define BSP_LCD_HOST            SPI2_HOST
#define BSP_LCD_PIXEL_CLOCK_HZ  (40 * 1000 * 1000)   // start at 40 MHz; can push to 80 MHz
#define BSP_LCD_BITS_PER_PIXEL  16
#define BSP_LCD_H_RES           320
#define BSP_LCD_V_RES           480

#define BSP_LCD_PIN_CS          45
#define BSP_LCD_PIN_SCK         47
#define BSP_LCD_PIN_D0          21
#define BSP_LCD_PIN_D1          48
#define BSP_LCD_PIN_D2          40
#define BSP_LCD_PIN_D3          39
#define BSP_LCD_PIN_RST         -1   // tied to NRST on this board
#define BSP_LCD_PIN_BL          1    // backlight (active high)

// ---- Touch: AXS15231B over I2C (shares chip with the LCD glass) ----
#define BSP_TOUCH_I2C_PORT      I2C_NUM_0
#define BSP_TOUCH_I2C_FREQ_HZ   400000
#define BSP_TOUCH_I2C_ADDR      0x3B
#define BSP_TOUCH_PIN_SDA       4
#define BSP_TOUCH_PIN_SCL       8
#define BSP_TOUCH_PIN_INT       3
#define BSP_TOUCH_PIN_RST       38

// ---- Sensors ----
// SHT3x / SHT31 air temp+humidity over I2C (default 0x44).
// Shares the touch I2C bus by default — wire SDA/SCL to the same lines.
#define BSP_SHT3X_I2C_ADDR      0x44

// BH1750 ambient light over I2C (0x23 with ADDR pin low, 0x5C with high).
#define BSP_BH1750_I2C_ADDR     0x23

// Capacitive soil moisture probe on ADC.
// ESP32-S3 ADC1 channels are on GPIO1..GPIO10. GPIO1 is the backlight here,
// so put the probe on GPIO5 by default. Update if your wiring differs.
#define BSP_SOIL_ADC_GPIO       5
#define BSP_SOIL_ADC_UNIT       ADC_UNIT_1
#define BSP_SOIL_ADC_CHANNEL    ADC_CHANNEL_4   // GPIO5 -> ADC1_CH4

// ---- Relays (irrigation / lights) ----
// Pick free GPIOs on the JC3248W535 break-out. These are sensible defaults —
// confirm they aren't claimed by the audio amp or SD-card mux on your variant.
#define BSP_RELAY_COUNT         2
#define BSP_RELAY_GPIOS         { 18, 17 }
#define BSP_RELAY_ACTIVE_HIGH   1
