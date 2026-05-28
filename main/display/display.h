#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_handle_t    panel;
} display_handles_t;

esp_err_t display_init(display_handles_t *out);
void      display_backlight(bool on);

#ifdef __cplusplus
}
#endif
