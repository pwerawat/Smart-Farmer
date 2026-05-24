#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "board.h"

typedef struct {
    float    air_temp_c;
    float    air_humidity_pct;
    uint16_t soil_raw;       // raw ADC counts
    float    soil_pct;       // 0..100 after calibration
    float    light_lux;
    bool     relay_on[BSP_RELAY_COUNT];
    uint32_t last_update_ms;
} app_state_t;

void app_state_init(void);
void app_state_get(app_state_t *out);
void app_state_update_sensors(float t_c, float rh_pct, uint16_t soil_raw, float soil_pct, float lux);
void app_state_set_relay(int idx, bool on);
