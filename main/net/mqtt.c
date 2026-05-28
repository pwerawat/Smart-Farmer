#include "mqtt.h"

#include <stdio.h>
#include <string.h>

#include "app_state.h"
#include "control/relays.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#ifndef MQTT_BROKER_URI
#define MQTT_BROKER_URI "mqtt://test.mosquitto.org"
#endif

static const char *TAG = "mqtt";
static esp_mqtt_client_handle_t s_client;
static char s_device_id[20];
static char s_telemetry_topic[64];
static char s_relay_sub_topic[64];

static void build_topics(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(s_device_id, sizeof(s_device_id), "%02x%02x%02x", mac[3], mac[4], mac[5]);
    snprintf(s_telemetry_topic, sizeof(s_telemetry_topic),
             "smart-farmer/%s/telemetry", s_device_id);
    snprintf(s_relay_sub_topic, sizeof(s_relay_sub_topic),
             "smart-farmer/%s/relay/+", s_device_id);
}

static void handle_relay_cmd(const char *topic, int topic_len,
                             const char *data, int data_len)
{
    // Topic ends with /relay/N; pull the N
    const char *last_slash = NULL;
    for (int i = topic_len - 1; i >= 0; i--) {
        if (topic[i] == '/') { last_slash = &topic[i]; break; }
    }
    if (!last_slash) return;
    int idx = atoi(last_slash + 1);

    bool on = (data_len >= 2 && (data[0] == 'o' || data[0] == 'O')
               && (data[1] == 'n' || data[1] == 'N'));
    relays_set(idx, on);
}

static void mqtt_event_handler(void *args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t e = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "connected, subscribing to %s", s_relay_sub_topic);
        esp_mqtt_client_subscribe(s_client, s_relay_sub_topic, 1);
        break;
    case MQTT_EVENT_DATA:
        handle_relay_cmd(e->topic, e->topic_len, e->data, e->data_len);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "disconnected");
        break;
    default:
        break;
    }
}

static void publish_task(void *arg)
{
    while (1) {
        app_state_t s;
        app_state_get(&s);

        char payload[192];
        int n = snprintf(payload, sizeof(payload),
            "{\"t\":%.2f,\"rh\":%.1f,\"soil\":%.1f,\"lux\":%.0f,"
            "\"r0\":%d,\"r1\":%d,\"ts\":%u}",
            s.air_temp_c, s.air_humidity_pct, s.soil_pct, s.light_lux,
            s.relay_on[0] ? 1 : 0,
            BSP_RELAY_COUNT > 1 ? (s.relay_on[1] ? 1 : 0) : 0,
            (unsigned)s.last_update_ms);
        if (n > 0) {
            esp_mqtt_client_publish(s_client, s_telemetry_topic, payload, n, 0, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

esp_err_t mqtt_start(void)
{
    build_topics();
    ESP_LOGI(TAG, "device-id=%s broker=%s", s_device_id, MQTT_BROKER_URI);

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.client_id = s_device_id,
    };
    s_client = esp_mqtt_client_init(&cfg);
    if (!s_client) return ESP_FAIL;
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    esp_err_t err = esp_mqtt_client_start(s_client);
    if (err != ESP_OK) return err;

    xTaskCreate(publish_task, "mqtt-pub", 4096, NULL, 3, NULL);
    return ESP_OK;
}
