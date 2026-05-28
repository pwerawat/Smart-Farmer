#include "display.h"

#include "axs15231b_panel.h"
#include "board.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"

static const char *TAG = "display";

static void backlight_init(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BSP_LCD_PIN_BL,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);
    gpio_set_level(BSP_LCD_PIN_BL, 0);
}

void display_backlight(bool on)
{
    gpio_set_level(BSP_LCD_PIN_BL, on ? 1 : 0);
}

esp_err_t display_init(display_handles_t *out)
{
    ESP_LOGI(TAG, "QSPI bus init");

    backlight_init();

    spi_bus_config_t bus = {
        .data0_io_num = BSP_LCD_PIN_D0,
        .data1_io_num = BSP_LCD_PIN_D1,
        .data2_io_num = BSP_LCD_PIN_D2,
        .data3_io_num = BSP_LCD_PIN_D3,
        .sclk_io_num  = BSP_LCD_PIN_SCK,
        .max_transfer_sz = BSP_LCD_H_RES * 80 * sizeof(uint16_t),
        .flags        = SPICOMMON_BUSFLAG_QUAD,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_HOST, &bus, SPI_DMA_CH_AUTO),
                        TAG, "spi bus init");

    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num         = BSP_LCD_PIN_CS,
        .dc_gpio_num         = -1,
        .spi_mode            = 0,
        .pclk_hz             = BSP_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth   = 10,
        .lcd_cmd_bits        = 32,
        .lcd_param_bits      = 8,
        .flags = {
            .quad_mode = 1,
        },
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_HOST,
                        &io_cfg, &out->io), TAG, "panel io");

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = BSP_LCD_PIN_RST,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_axs15231b(out->io, &panel_cfg, &out->panel),
                        TAG, "panel create");

    esp_lcd_panel_reset(out->panel);
    esp_lcd_panel_init(out->panel);
    esp_lcd_panel_disp_on_off(out->panel, true);
    display_backlight(true);

    ESP_LOGI(TAG, "display ready (%dx%d)", BSP_LCD_H_RES, BSP_LCD_V_RES);
    return ESP_OK;
}
