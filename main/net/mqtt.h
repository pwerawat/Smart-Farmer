#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Connect to the broker (URI from MQTT_BROKER_URI, defaults to mqtt://test.mosquitto.org)
// and start a publisher task that pushes telemetry every 5 s on
//   smart-farmer/<device-id>/telemetry  (JSON)
// and subscribes to
//   smart-farmer/<device-id>/relay/+    (payload "on" / "off")
esp_err_t mqtt_start(void);

#ifdef __cplusplus
}
#endif
