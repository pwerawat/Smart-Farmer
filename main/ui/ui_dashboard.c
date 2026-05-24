#include "ui_dashboard.h"

#include <stdio.h>

#include "app_state.h"
#include "control/relays.h"
#include "lvgl.h"

typedef struct {
    lv_obj_t *temp_val;
    lv_obj_t *hum_val;
    lv_obj_t *soil_val;
    lv_obj_t *light_val;
    lv_obj_t *soil_bar;
    lv_obj_t *relay_btn[4];
    lv_obj_t *status;
} dash_widgets_t;

static dash_widgets_t W;

static lv_obj_t *make_card(lv_obj_t *parent, const char *title, int x, int y, int w, int h)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_pad_all(card, 10, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1d2330), 0);
    lv_obj_set_style_border_width(card, 0, 0);

    lv_obj_t *lbl = lv_label_create(card);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x8aa0bf), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 0, 0);

    return card;
}

static void relay_event_cb(lv_event_t *e)
{
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target(e);
    bool on = lv_obj_has_state(btn, LV_STATE_CHECKED);
    relays_set(idx, on);
}

static void refresh_cb(lv_timer_t *t)
{
    app_state_t s;
    app_state_get(&s);

    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f °C", s.air_temp_c);
    lv_label_set_text(W.temp_val, buf);
    snprintf(buf, sizeof(buf), "%.0f %%", s.air_humidity_pct);
    lv_label_set_text(W.hum_val, buf);
    snprintf(buf, sizeof(buf), "%.0f %%", s.soil_pct);
    lv_label_set_text(W.soil_val, buf);
    snprintf(buf, sizeof(buf), "%.0f lx", s.light_lux);
    lv_label_set_text(W.light_val, buf);

    lv_bar_set_value(W.soil_bar, (int32_t)s.soil_pct, LV_ANIM_OFF);

    for (int i = 0; i < BSP_RELAY_COUNT && i < 4; i++) {
        if (s.relay_on[i]) lv_obj_add_state(W.relay_btn[i], LV_STATE_CHECKED);
        else               lv_obj_remove_state(W.relay_btn[i], LV_STATE_CHECKED);
    }
}

void ui_dashboard_create(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x10141c), 0);

    // Header
    lv_obj_t *hdr = lv_label_create(scr);
    lv_label_set_text(hdr, "Smart Farmer");
    lv_obj_set_style_text_color(hdr, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(hdr, &lv_font_montserrat_20, 0);
    lv_obj_align(hdr, LV_ALIGN_TOP_LEFT, 12, 10);

    W.status = lv_label_create(scr);
    lv_label_set_text(W.status, "boot");
    lv_obj_set_style_text_color(W.status, lv_color_hex(0x66c266), 0);
    lv_obj_align(W.status, LV_ALIGN_TOP_RIGHT, -12, 14);

    // 4 sensor cards in a 2x2 grid below the header
    int cw = 145, ch = 90, gx = 12, gy = 50;

    lv_obj_t *c_temp = make_card(scr, "Air temp",  gx,              gy,             cw, ch);
    lv_obj_t *c_hum  = make_card(scr, "Humidity",  gx + cw + 8,     gy,             cw, ch);
    lv_obj_t *c_soil = make_card(scr, "Soil",      gx,              gy + ch + 8,    cw, ch);
    lv_obj_t *c_lux  = make_card(scr, "Light",     gx + cw + 8,     gy + ch + 8,    cw, ch);

    W.temp_val  = lv_label_create(c_temp);
    W.hum_val   = lv_label_create(c_hum);
    W.soil_val  = lv_label_create(c_soil);
    W.light_val = lv_label_create(c_lux);

    lv_obj_t *vals[] = { W.temp_val, W.hum_val, W.soil_val, W.light_val };
    for (int i = 0; i < 4; i++) {
        lv_obj_set_style_text_color(vals[i], lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(vals[i], &lv_font_montserrat_24, 0);
        lv_obj_align(vals[i], LV_ALIGN_BOTTOM_LEFT, 0, -4);
    }

    // Soil bar (full-width strip)
    W.soil_bar = lv_bar_create(scr);
    lv_obj_set_size(W.soil_bar, 296, 12);
    lv_obj_align(W.soil_bar, LV_ALIGN_TOP_LEFT, 12, gy + 2 * ch + 28);
    lv_bar_set_range(W.soil_bar, 0, 100);
    lv_obj_set_style_bg_color(W.soil_bar, lv_color_hex(0x222831), 0);
    lv_obj_set_style_bg_color(W.soil_bar, lv_color_hex(0x4fc3f7), LV_PART_INDICATOR);

    // Relay row
    lv_obj_t *relays_lbl = lv_label_create(scr);
    lv_label_set_text(relays_lbl, "Relays");
    lv_obj_set_style_text_color(relays_lbl, lv_color_hex(0x8aa0bf), 0);
    lv_obj_align(relays_lbl, LV_ALIGN_TOP_LEFT, 12, gy + 2 * ch + 50);

    int by = gy + 2 * ch + 76;
    int bw = (BSP_LCD_H_RES - 24 - (BSP_RELAY_COUNT - 1) * 8) / BSP_RELAY_COUNT;
    for (int i = 0; i < BSP_RELAY_COUNT && i < 4; i++) {
        lv_obj_t *btn = lv_btn_create(scr);
        lv_obj_set_size(btn, bw, 60);
        lv_obj_set_pos(btn, 12 + i * (bw + 8), by);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(btn, relay_event_cb, LV_EVENT_VALUE_CHANGED,
                            (void *)(intptr_t)i);
        lv_obj_t *l = lv_label_create(btn);
        lv_label_set_text_fmt(l, "Relay %d", i + 1);
        lv_obj_center(l);
        W.relay_btn[i] = btn;
    }

    // Refresh from app_state every 500 ms
    lv_timer_create(refresh_cb, 500, NULL);
}
