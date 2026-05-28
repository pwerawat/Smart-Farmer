#pragma once

#include "display.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Start LVGL: create display + input device wired to the AXS15231B and spawn
// the LVGL tick + handler tasks. Must be called once after display_init().
esp_err_t lvgl_port_start(const display_handles_t *display);

// Take/release the LVGL mutex around any lv_* call from outside the LVGL task.
void lvgl_port_lock(void);
void lvgl_port_unlock(void);

#ifdef __cplusplus
}
#endif
