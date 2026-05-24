#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Bring up Wi-Fi in station mode. SSID/password come from the build-time
// macros CONFIG_WIFI_SSID / CONFIG_WIFI_PASS (set them in sdkconfig or
// via -DWIFI_SSID=..., -DWIFI_PASS=...). Returns once IP is acquired or
// the join times out; check the return code.
esp_err_t wifi_start_blocking(void);

bool wifi_is_connected(void);

#ifdef __cplusplus
}
#endif
