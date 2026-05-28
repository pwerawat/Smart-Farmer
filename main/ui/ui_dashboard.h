#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Build the dashboard on the active LVGL screen. Caller must hold the LVGL
// mutex. Subsequent updates happen automatically from a timer reading
// app_state_get().
void ui_dashboard_create(void);

#ifdef __cplusplus
}
#endif
