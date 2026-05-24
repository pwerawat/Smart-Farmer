#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t axs15231b_touch_init(void);

// Returns true and writes coords if a touch is present. (0,0) origin = top-left
// in the panel's natural orientation. Caller maps to LVGL orientation.
bool axs15231b_touch_read(uint16_t *x, uint16_t *y);

#ifdef __cplusplus
}
#endif
