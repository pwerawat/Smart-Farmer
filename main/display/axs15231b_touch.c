// Bare-minimum I2C driver for the AXS15231B touch controller.
//
// The chip reports a single touch point in a fixed 8-byte burst beginning at
// register 0x01. Byte 0 holds the gesture / fingers count, bytes 1..4 hold
// X(hi/lo) and Y(hi/lo). Multi-touch is supported by the chip but not wired
// here — LVGL only needs one point for our dashboard.

#include "axs15231b_touch.h"

#include <string.h>

#include "board.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "axs-touch";

static esp_err_t reg_read(uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_write_read_device(BSP_TOUCH_I2C_PORT,
                                        BSP_TOUCH_I2C_ADDR,
                                        &reg, 1, buf, len,
                                        pdMS_TO_TICKS(50));
}

esp_err_t axs15231b_touch_init(void)
{
    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = BSP_TOUCH_PIN_SDA,
        .scl_io_num       = BSP_TOUCH_PIN_SCL,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = BSP_TOUCH_I2C_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(BSP_TOUCH_I2C_PORT, &conf);
    if (err != ESP_OK) return err;
    err = i2c_driver_install(BSP_TOUCH_I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

    if (BSP_TOUCH_PIN_RST >= 0) {
        gpio_config_t rst = {
            .pin_bit_mask = 1ULL << BSP_TOUCH_PIN_RST,
            .mode = GPIO_MODE_OUTPUT,
        };
        gpio_config(&rst);
        gpio_set_level(BSP_TOUCH_PIN_RST, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(BSP_TOUCH_PIN_RST, 1);
        vTaskDelay(pdMS_TO_TICKS(120));
    }

    ESP_LOGI(TAG, "AXS15231B touch ready");
    return ESP_OK;
}

bool axs15231b_touch_read(uint16_t *x, uint16_t *y)
{
    uint8_t buf[8] = {0};
    if (reg_read(0x01, buf, sizeof(buf)) != ESP_OK) return false;

    uint8_t fingers = buf[1] & 0x0F;
    if (fingers == 0) return false;

    *x = ((uint16_t)(buf[2] & 0x0F) << 8) | buf[3];
    *y = ((uint16_t)(buf[4] & 0x0F) << 8) | buf[5];
    return true;
}
