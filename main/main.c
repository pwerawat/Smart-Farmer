#include "app_state.h"
#include "control/relays.h"
#include "display/axs15231b_touch.h"
#include "display/display.h"
#include "display/lvgl_port.h"
#include "esp_log.h"
#include "net/mqtt.h"
#include "net/wifi.h"
#include "sensors/sensors.h"
#include "ui/ui_dashboard.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Smart-Farmer boot");

    app_state_init();
    relays_init();

    display_handles_t disp;
    ESP_ERROR_CHECK(display_init(&disp));
    ESP_ERROR_CHECK(axs15231b_touch_init());
    ESP_ERROR_CHECK(lvgl_port_start(&disp));

    lvgl_port_lock();
    ui_dashboard_create();
    lvgl_port_unlock();

    sensors_start();

    if (wifi_start_blocking() == ESP_OK) {
        mqtt_start();
    } else {
        ESP_LOGW(TAG, "Wi-Fi unavailable — continuing in offline mode");
    }

    ESP_LOGI(TAG, "boot complete");
}
