#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialise SHT3x + BH1750 (shared I2C bus with the touch panel) and the
// soil-moisture ADC. Spawns a background task that polls every ~2 s and
// pushes readings into app_state.
esp_err_t sensors_start(void);

#ifdef __cplusplus
}
#endif
