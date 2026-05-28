#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t relays_init(void);
void      relays_set(int idx, bool on);
bool      relays_get(int idx);

#ifdef __cplusplus
}
#endif
