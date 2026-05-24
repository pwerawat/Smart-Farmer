#include "sensors.h"

#include <string.h>

#include "app_state.h"
#include "board.h"
#include "driver/i2c.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "sensors";

// ---- SHT3x ----
static esp_err_t sht3x_read(float *t_c, float *rh_pct)
{
    uint8_t cmd[2] = {0x2C, 0x06}; // single shot, high repeatability, clock-stretching
    esp_err_t err = i2c_master_write_to_device(BSP_TOUCH_I2C_PORT, BSP_SHT3X_I2C_ADDR,
                                               cmd, sizeof(cmd), pdMS_TO_TICKS(50));
    if (err != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t buf[6];
    err = i2c_master_read_from_device(BSP_TOUCH_I2C_PORT, BSP_SHT3X_I2C_ADDR,
                                      buf, sizeof(buf), pdMS_TO_TICKS(50));
    if (err != ESP_OK) return err;

    uint16_t raw_t = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t raw_h = ((uint16_t)buf[3] << 8) | buf[4];
    *t_c    = -45.0f + 175.0f * ((float)raw_t / 65535.0f);
    *rh_pct = 100.0f * ((float)raw_h / 65535.0f);
    return ESP_OK;
}

// ---- BH1750 ----
static esp_err_t bh1750_init(void)
{
    uint8_t cmd = 0x10; // Continuous H-Resolution mode (1 lx, ~120 ms)
    return i2c_master_write_to_device(BSP_TOUCH_I2C_PORT, BSP_BH1750_I2C_ADDR,
                                      &cmd, 1, pdMS_TO_TICKS(50));
}

static esp_err_t bh1750_read(float *lux)
{
    uint8_t buf[2];
    esp_err_t err = i2c_master_read_from_device(BSP_TOUCH_I2C_PORT, BSP_BH1750_I2C_ADDR,
                                                buf, sizeof(buf), pdMS_TO_TICKS(50));
    if (err != ESP_OK) return err;
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    *lux = raw / 1.2f;
    return ESP_OK;
}

// ---- Capacitive soil probe (ADC) ----
// Most cheap capacitive probes report ~3.0 V dry and ~1.4 V fully wet at 3v3.
// Adjust these once you've calibrated against your soil.
#define SOIL_DRY_RAW   2900
#define SOIL_WET_RAW   1300

static adc_oneshot_unit_handle_t s_adc;

static esp_err_t soil_init(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = BSP_SOIL_ADC_UNIT,
    };
    esp_err_t err = adc_oneshot_new_unit(&unit_cfg, &s_adc);
    if (err != ESP_OK) return err;

    adc_oneshot_chan_cfg_t ch_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    return adc_oneshot_config_channel(s_adc, BSP_SOIL_ADC_CHANNEL, &ch_cfg);
}

static uint16_t soil_read_raw(void)
{
    int raw = 0;
    if (adc_oneshot_read(s_adc, BSP_SOIL_ADC_CHANNEL, &raw) != ESP_OK) return 0;
    return (uint16_t)raw;
}

static float soil_raw_to_pct(uint16_t raw)
{
    if (raw >= SOIL_DRY_RAW) return 0.0f;
    if (raw <= SOIL_WET_RAW) return 100.0f;
    return 100.0f * (float)(SOIL_DRY_RAW - raw) / (float)(SOIL_DRY_RAW - SOIL_WET_RAW);
}

// ---- Polling task ----
static void sensors_task(void *arg)
{
    if (bh1750_init() != ESP_OK) {
        ESP_LOGW(TAG, "BH1750 init failed (continuing)");
    }

    while (1) {
        float t = 0, rh = 0, lux = 0;
        if (sht3x_read(&t, &rh) != ESP_OK) {
            ESP_LOGW(TAG, "SHT3x read failed");
            t = rh = 0;
        }
        if (bh1750_read(&lux) != ESP_OK) {
            lux = 0;
        }
        uint16_t soil_raw = soil_read_raw();
        float    soil_pct = soil_raw_to_pct(soil_raw);

        app_state_update_sensors(t, rh, soil_raw, soil_pct, lux);
        ESP_LOGI(TAG, "T=%.1fC RH=%.0f%% soil=%u(%.0f%%) lux=%.0f",
                 t, rh, soil_raw, soil_pct, lux);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

esp_err_t sensors_start(void)
{
    if (soil_init() != ESP_OK) {
        ESP_LOGE(TAG, "Soil ADC init failed");
        return ESP_FAIL;
    }
    xTaskCreate(sensors_task, "sensors", 4096, NULL, 4, NULL);
    return ESP_OK;
}
