#include "relays.h"

#include "app_state.h"
#include "board.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "relays";

static const int s_pins[] = BSP_RELAY_GPIOS;

esp_err_t relays_init(void)
{
    uint64_t mask = 0;
    for (int i = 0; i < BSP_RELAY_COUNT; i++) {
        mask |= 1ULL << s_pins[i];
    }
    gpio_config_t io = {
        .pin_bit_mask = mask,
        .mode = GPIO_MODE_OUTPUT,
    };
    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) return err;

    for (int i = 0; i < BSP_RELAY_COUNT; i++) {
        gpio_set_level(s_pins[i], BSP_RELAY_ACTIVE_HIGH ? 0 : 1);
        app_state_set_relay(i, false);
    }
    ESP_LOGI(TAG, "%d relay(s) ready", BSP_RELAY_COUNT);
    return ESP_OK;
}

void relays_set(int idx, bool on)
{
    if (idx < 0 || idx >= BSP_RELAY_COUNT) return;
    int level = on ? BSP_RELAY_ACTIVE_HIGH : !BSP_RELAY_ACTIVE_HIGH;
    gpio_set_level(s_pins[idx], level);
    app_state_set_relay(idx, on);
    ESP_LOGI(TAG, "relay[%d] = %s", idx, on ? "on" : "off");
}

bool relays_get(int idx)
{
    app_state_t s;
    app_state_get(&s);
    if (idx < 0 || idx >= BSP_RELAY_COUNT) return false;
    return s.relay_on[idx];
}
