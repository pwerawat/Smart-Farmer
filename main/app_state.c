#include "app_state.h"

#include <string.h>

#include "esp_timer.h"

static app_state_t   s_state;
static SemaphoreHandle_t s_lock;

void app_state_init(void)
{
    memset(&s_state, 0, sizeof(s_state));
    s_lock = xSemaphoreCreateMutex();
}

void app_state_get(app_state_t *out)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    *out = s_state;
    xSemaphoreGive(s_lock);
}

void app_state_update_sensors(float t_c, float rh_pct, uint16_t soil_raw, float soil_pct, float lux)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_state.air_temp_c      = t_c;
    s_state.air_humidity_pct = rh_pct;
    s_state.soil_raw        = soil_raw;
    s_state.soil_pct        = soil_pct;
    s_state.light_lux       = lux;
    s_state.last_update_ms  = (uint32_t)(esp_timer_get_time() / 1000);
    xSemaphoreGive(s_lock);
}

void app_state_set_relay(int idx, bool on)
{
    if (idx < 0 || idx >= BSP_RELAY_COUNT) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_state.relay_on[idx] = on;
    xSemaphoreGive(s_lock);
}
