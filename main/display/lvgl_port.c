#include "lvgl_port.h"

#include "axs15231b_touch.h"
#include "board.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"

static const char *TAG = "lvgl";

#define LVGL_TICK_PERIOD_MS  2
#define LVGL_TASK_STACK      6144
#define LVGL_TASK_PRIO       2
#define LVGL_DRAW_BUF_LINES  40   // 40 lines * 320 px * 2 bytes = 25.6 KB per buf

static SemaphoreHandle_t        s_lock;
static esp_lcd_panel_handle_t   s_panel;
static lv_display_t            *s_disp;
static lv_indev_t              *s_indev;

void lvgl_port_lock(void)   { xSemaphoreTakeRecursive(s_lock, portMAX_DELAY); }
void lvgl_port_unlock(void) { xSemaphoreGiveRecursive(s_lock); }

static void tick_cb(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_draw_bitmap(s_panel,
                              area->x1, area->y1,
                              area->x2 + 1, area->y2 + 1,
                              px_map);
    // esp_lcd transfers are async-by-queue but draw_bitmap blocks until queued.
    lv_display_flush_ready(disp);
}

static void touch_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    uint16_t x = 0, y = 0;
    if (axs15231b_touch_read(&x, &y)) {
        if (x >= BSP_LCD_H_RES) x = BSP_LCD_H_RES - 1;
        if (y >= BSP_LCD_V_RES) y = BSP_LCD_V_RES - 1;
        data->point.x = x;
        data->point.y = y;
        data->state   = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void lvgl_task(void *arg)
{
    ESP_LOGI(TAG, "LVGL task running");
    while (1) {
        lvgl_port_lock();
        uint32_t delay = lv_timer_handler();
        lvgl_port_unlock();
        if (delay > 50) delay = 50;
        vTaskDelay(pdMS_TO_TICKS(delay ? delay : 5));
    }
}

esp_err_t lvgl_port_start(const display_handles_t *display)
{
    s_panel = display->panel;
    s_lock  = xSemaphoreCreateRecursiveMutex();

    lv_init();

    // Two partial buffers in PSRAM-friendly DMA-capable region.
    size_t buf_px = BSP_LCD_H_RES * LVGL_DRAW_BUF_LINES;
    void *buf1 = heap_caps_malloc(buf_px * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    void *buf2 = heap_caps_malloc(buf_px * 2, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!buf1 || !buf2) {
        ESP_LOGE(TAG, "LVGL draw buffer alloc failed");
        return ESP_ERR_NO_MEM;
    }

    s_disp = lv_display_create(BSP_LCD_H_RES, BSP_LCD_V_RES);
    lv_display_set_color_format(s_disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(s_disp, buf1, buf2, buf_px * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_disp, flush_cb);

    s_indev = lv_indev_create();
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_indev, touch_cb);

    const esp_timer_create_args_t tick_args = {
        .callback = tick_cb,
        .name     = "lv_tick",
    };
    esp_timer_handle_t tick_timer = NULL;
    esp_timer_create(&tick_args, &tick_timer);
    esp_timer_start_periodic(tick_timer, LVGL_TICK_PERIOD_MS * 1000);

    xTaskCreatePinnedToCore(lvgl_task, "lvgl", LVGL_TASK_STACK, NULL,
                            LVGL_TASK_PRIO, NULL, 1);
    return ESP_OK;
}
