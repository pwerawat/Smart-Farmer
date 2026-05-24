# LVGL dashboard. Two tabs: Sensors and Relays.

try:
    import lvgl as lv
except ImportError:
    lv = None


class Dashboard:
    def __init__(self, relays):
        if lv is None:
            raise ImportError("LVGL not available")
        self.relays = relays
        self._build()

    def _build(self):
        scr = lv.screen_active()
        scr.set_style_bg_color(lv.color_hex(0x101418), 0)

        self.tabs = lv.tabview(scr)
        self.tabs.set_size(lv.pct(100), lv.pct(100))
        self.tabs.set_tab_bar_size(40)

        sensors_tab = self.tabs.add_tab("Sensors")
        relays_tab = self.tabs.add_tab("Relays")
        status_tab = self.tabs.add_tab("Status")

        self._build_sensors(sensors_tab)
        self._build_relays(relays_tab)
        self._build_status(status_tab)

    def _label(self, parent, text, x, y, big=False):
        lbl = lv.label(parent)
        lbl.set_text(text)
        lbl.set_pos(x, y)
        lbl.set_style_text_color(lv.color_hex(0xE8E8E8), 0)
        if big:
            lbl.set_style_text_font(lv.font_montserrat_28, 0)
        return lbl

    def _build_sensors(self, parent):
        self.lbl_temp = self._label(parent, "-- C", 20, 20, big=True)
        self._label(parent, "Temperature", 20, 60)

        self.lbl_humidity = self._label(parent, "-- %", 20, 110, big=True)
        self._label(parent, "Humidity", 20, 150)

        self.lbl_soil = self._label(parent, "-- %", 20, 200, big=True)
        self._label(parent, "Soil moisture", 20, 240)

        self.lbl_light = self._label(parent, "--", 20, 290, big=True)
        self._label(parent, "Light", 20, 330)

    def _build_relays(self, parent):
        self._relay_switches = {}
        y = 10
        for r in self.relays:
            row = lv.obj(parent)
            row.set_size(lv.pct(100), 56)
            row.set_pos(0, y)
            row.set_style_bg_color(lv.color_hex(0x1B2026), 0)
            row.set_style_pad_all(8, 0)

            name = lv.label(row)
            name.set_text(r.label)
            name.set_style_text_color(lv.color_hex(0xE8E8E8), 0)
            name.align(lv.ALIGN.LEFT_MID, 8, 0)

            sw = lv.switch(row)
            sw.align(lv.ALIGN.RIGHT_MID, -8, 0)
            sw.add_event_cb(
                lambda e, relay=r: relay.set(e.get_target_obj().has_state(lv.STATE.CHECKED)),
                lv.EVENT.VALUE_CHANGED, None,
            )
            self._relay_switches[r.label] = sw
            y += 64

    def _build_status(self, parent):
        self.lbl_wifi = self._label(parent, "Wi-Fi: --", 20, 20)
        self.lbl_mqtt = self._label(parent, "MQTT: --", 20, 60)
        self.lbl_uptime = self._label(parent, "Uptime: --", 20, 100)
        self.lbl_ip = self._label(parent, "IP: --", 20, 140)

    def update_sensors(self, reading):
        t = reading.get("temp_c")
        h = reading.get("humidity_pct")
        s = reading.get("soil_pct") or []
        self.lbl_temp.set_text("{:.1f} C".format(t) if t is not None else "-- C")
        self.lbl_humidity.set_text("{:.0f} %".format(h) if h is not None else "-- %")
        self.lbl_soil.set_text("{} %".format(s[0]) if s else "-- %")
        self.lbl_light.set_text(str(reading.get("light_raw", "--")))

    def update_relays(self):
        for label, sw in self._relay_switches.items():
            r = self.relays.get(label)
            if not r:
                continue
            if r.is_on() and not sw.has_state(lv.STATE.CHECKED):
                sw.add_state(lv.STATE.CHECKED)
            elif not r.is_on() and sw.has_state(lv.STATE.CHECKED):
                sw.clear_state(lv.STATE.CHECKED)

    def update_status(self, wifi_ok, mqtt_ok, ip, uptime_s):
        self.lbl_wifi.set_text("Wi-Fi: " + ("connected" if wifi_ok else "offline"))
        self.lbl_mqtt.set_text("MQTT: " + ("connected" if mqtt_ok else "offline"))
        self.lbl_ip.set_text("IP: " + (ip or "--"))
        self.lbl_uptime.set_text("Uptime: {}s".format(uptime_s))
