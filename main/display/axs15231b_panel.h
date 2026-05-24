#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an esp_lcd panel handle for the Guition AXS15231B QSPI LCD.
 *
 * @param io           Panel-IO handle (must be a QSPI handle created via
 *                     esp_lcd_new_panel_io_spi with quad mode + 32-bit commands).
 * @param panel_config Vendor panel config (reset GPIO, color space, etc.).
 * @param ret_panel    Returned panel handle.
 */
esp_err_t esp_lcd_new_panel_axs15231b(esp_lcd_panel_io_handle_t io,
                                      const esp_lcd_panel_dev_config_t *panel_config,
                                      esp_lcd_panel_handle_t *ret_panel);

#ifdef __cplusplus
}
#endif
