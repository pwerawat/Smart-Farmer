// Minimal esp_lcd panel driver for the AXS15231B QSPI LCD on JC3248W535.
//
// The AXS15231B speaks a custom QSPI framing where every command word is
// 32 bits: { 0x02, cmd_high, cmd_low, 0x00 } for writes (single-line cmd,
// 4-line data follows for pixels via the 0x32 prefix). The esp_lcd SPI
// panel-IO supports this when configured with .lcd_cmd_bits = 32 and
// .flags.quad_mode = 1.
//
// Init sequence is taken from the Guition reference firmware. If your
// board variant ships with a different OTP, tweak s_init_cmds[].

#include "axs15231b_panel.h"

#include <string.h>

#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "axs15231b";

// AXS15231B-specific opcodes
#define AXS_CMD_SLPOUT   0x11
#define AXS_CMD_DISPON   0x29
#define AXS_CMD_CASET    0x2A
#define AXS_CMD_RASET    0x2B
#define AXS_CMD_RAMWR    0x2C
#define AXS_CMD_MADCTL   0x36
#define AXS_CMD_COLMOD   0x3A

// QSPI framing prefixes. The chip expects every 32-bit command to be
// { 0x02, 0x00, cmd, 0x00 } for parameter writes and
// { 0x32, 0x00, 0x2C, 0x00 } for pixel writes (RAMWR over 4-line data).
#define AXS_QSPI_WRITE_PARAM(cmd)  (((uint32_t)0x02 << 24) | ((uint32_t)(cmd) << 8))
#define AXS_QSPI_WRITE_PIXEL       (((uint32_t)0x32 << 24) | ((uint32_t)0x2C << 8))

static inline esp_err_t axs_tx_param(esp_lcd_panel_io_handle_t io,
                                     uint8_t cmd, const void *param, size_t len)
{
    return esp_lcd_panel_io_tx_param(io, AXS_QSPI_WRITE_PARAM(cmd), param, len);
}

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int   reset_gpio_num;
    bool  reset_level;
    uint8_t madctl_val;
    uint8_t colmod_val;
    int   x_gap;
    int   y_gap;
} axs_panel_t;

typedef struct {
    uint8_t cmd;
    uint8_t data[20];
    uint8_t data_bytes;   // 0x80 == end marker; bit7 set on data_bytes means "delay 120 ms after"
} lcd_init_cmd_t;

// Vendor init sequence. The values come from Guition's reference; if your
// board ships a different panel OTP, replace this table.
static const lcd_init_cmd_t s_init_cmds[] = {
    {0xBB, {0x5A, 0xA5}, 2},
    {0xA0, {0xC0, 0x10, 0x00, 0x02, 0x00, 0x00, 0x04, 0x3F,
            0x20, 0x05, 0x0F, 0x18, 0x21, 0x10}, 14},
    {0xA2, {0x30, 0x19, 0x1A, 0x19, 0x1E, 0x05, 0x22, 0x25,
            0x14, 0x19, 0x33, 0x0E, 0x06, 0x10, 0x05, 0x0A, 0xFC}, 17},
    {0xD0, {0x07, 0xFF, 0xFF}, 3},
    {0xA3, {0xEE, 0x70, 0x10, 0x10, 0x10, 0x10, 0x06, 0x06, 0x06, 0x06}, 10},
    {0xA8, {0xBB, 0x76}, 2},
    {0xB6, {0xB3}, 1},
    {0xB7, {0xC0}, 1},
    {0xBA, {0x01}, 1},
    {0xBF, {0x40}, 1},
    {0xC1, {0x0D, 0x02, 0x07, 0x07, 0x07, 0x07}, 6},
    {0xC2, {0x07}, 1},
    {0xC3, {0x00, 0x00, 0x00, 0x80}, 4},
    {0xCD, {0x00}, 1},
    {0xF0, {0x12}, 1},
    {0xF1, {0x10}, 1},
    {0xF2, {0xC0}, 1},
    {0x35, {0x00}, 1},                  // TE on
    {0x36, {0x00}, 1},                  // MADCTL — overridden by user config
    {0x3A, {0x05}, 1},                  // COLMOD = 16bpp
    {0x11, {0}, 0x80},                  // sleep out + 120 ms delay
    {0x29, {0}, 0x80},                  // display on + 120 ms delay
};

static esp_err_t axs_reset(esp_lcd_panel_t *panel);
static esp_err_t axs_init(esp_lcd_panel_t *panel);
static esp_err_t axs_del(esp_lcd_panel_t *panel);
static esp_err_t axs_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start,
                                 int x_end, int y_end, const void *color_data);
static esp_err_t axs_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t axs_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t axs_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t axs_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t axs_disp_on_off(esp_lcd_panel_t *panel, bool on);

esp_err_t esp_lcd_new_panel_axs15231b(esp_lcd_panel_io_handle_t io,
                                      const esp_lcd_panel_dev_config_t *panel_config,
                                      esp_lcd_panel_handle_t *ret_panel)
{
    ESP_RETURN_ON_FALSE(io && panel_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "bad args");

    axs_panel_t *p = calloc(1, sizeof(*p));
    ESP_RETURN_ON_FALSE(p, ESP_ERR_NO_MEM, TAG, "no mem");

    if (panel_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << panel_config->reset_gpio_num,
            .mode = GPIO_MODE_OUTPUT,
        };
        ESP_ERROR_CHECK(gpio_config(&io_conf));
    }

    p->io             = io;
    p->reset_gpio_num = panel_config->reset_gpio_num;
    p->reset_level    = panel_config->flags.reset_active_high;
    p->colmod_val     = 0x55;   // 16bpp
    p->madctl_val     = 0x00;

    if (panel_config->rgb_ele_order == LCD_RGB_ELEMENT_ORDER_BGR) {
        p->madctl_val |= (1 << 3);
    }

    p->base.del          = axs_del;
    p->base.reset        = axs_reset;
    p->base.init         = axs_init;
    p->base.draw_bitmap  = axs_draw_bitmap;
    p->base.invert_color = axs_invert_color;
    p->base.set_gap      = axs_set_gap;
    p->base.mirror       = axs_mirror;
    p->base.swap_xy      = axs_swap_xy;
    p->base.disp_on_off  = axs_disp_on_off;

    *ret_panel = &p->base;
    return ESP_OK;
}

static esp_err_t axs_del(esp_lcd_panel_t *panel)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);
    free(p);
    return ESP_OK;
}

static esp_err_t axs_reset(esp_lcd_panel_t *panel)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);
    if (p->reset_gpio_num >= 0) {
        gpio_set_level(p->reset_gpio_num, p->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(p->reset_gpio_num, !p->reset_level);
        vTaskDelay(pdMS_TO_TICKS(120));
    } else {
        // Software reset
        axs_tx_param(p->io, 0x01, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(120));
    }
    return ESP_OK;
}

static esp_err_t axs_init(esp_lcd_panel_t *panel)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);

    for (size_t i = 0; i < sizeof(s_init_cmds) / sizeof(s_init_cmds[0]); i++) {
        const lcd_init_cmd_t *c = &s_init_cmds[i];
        uint8_t len = c->data_bytes & 0x1F;
        ESP_RETURN_ON_ERROR(axs_tx_param(p->io, c->cmd, c->data, len),
                            TAG, "init cmd 0x%02x failed", c->cmd);
        if (c->data_bytes & 0x80) {
            vTaskDelay(pdMS_TO_TICKS(120));
        }
    }

    // Apply user-selected MADCTL / COLMOD (re-issue in case init table set defaults)
    ESP_RETURN_ON_ERROR(axs_tx_param(p->io, AXS_CMD_MADCTL, (uint8_t[]){p->madctl_val}, 1), TAG, "MADCTL");
    ESP_RETURN_ON_ERROR(axs_tx_param(p->io, AXS_CMD_COLMOD,
                        (uint8_t[]){p->colmod_val}, 1), TAG, "COLMOD");

    ESP_LOGI(TAG, "AXS15231B init done");
    return ESP_OK;
}

static esp_err_t axs_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start,
                                 int x_end, int y_end, const void *color_data)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);

    x_start += p->x_gap;
    x_end   += p->x_gap;
    y_start += p->y_gap;
    y_end   += p->y_gap;

    uint8_t caset[4] = {
        (x_start >> 8) & 0xFF, x_start & 0xFF,
        ((x_end - 1) >> 8) & 0xFF, (x_end - 1) & 0xFF,
    };
    uint8_t raset[4] = {
        (y_start >> 8) & 0xFF, y_start & 0xFF,
        ((y_end - 1) >> 8) & 0xFF, (y_end - 1) & 0xFF,
    };

    axs_tx_param(p->io, AXS_CMD_CASET, caset, sizeof(caset));
    axs_tx_param(p->io, AXS_CMD_RASET, raset, sizeof(raset));

    size_t bytes = (size_t)(x_end - x_start) * (y_end - y_start) * 2;
    return esp_lcd_panel_io_tx_color(p->io, AXS_QSPI_WRITE_PIXEL, color_data, bytes);
}

static esp_err_t axs_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);
    return axs_tx_param(p->io, invert_color_data ? 0x21 : 0x20, NULL, 0);
}

static esp_err_t axs_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);
    if (mirror_x) p->madctl_val |= (1 << 6); else p->madctl_val &= ~(1 << 6);
    if (mirror_y) p->madctl_val |= (1 << 7); else p->madctl_val &= ~(1 << 7);
    return axs_tx_param(p->io, AXS_CMD_MADCTL, &p->madctl_val, 1);
}

static esp_err_t axs_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);
    if (swap_axes) p->madctl_val |= (1 << 5); else p->madctl_val &= ~(1 << 5);
    return axs_tx_param(p->io, AXS_CMD_MADCTL, &p->madctl_val, 1);
}

static esp_err_t axs_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);
    p->x_gap = x_gap;
    p->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t axs_disp_on_off(esp_lcd_panel_t *panel, bool on)
{
    axs_panel_t *p = __containerof(panel, axs_panel_t, base);
    return axs_tx_param(p->io, on ? AXS_CMD_DISPON : 0x28, NULL, 0);
}
