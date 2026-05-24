from machine import ADC, Pin
import time

import config

try:
    import dht
except ImportError:
    dht = None


class SensorBank:
    def __init__(self):
        self._dht = dht.DHT22(Pin(config.DHT_PIN)) if dht else None
        self._soil = [ADC(Pin(p), atten=ADC.ATTN_11DB) for p in config.SOIL_ADC_PINS]
        self._light = ADC(Pin(config.LIGHT_ADC_PIN), atten=ADC.ATTN_11DB)
        self._last_dht_ms = 0
        self._cached = {"temp_c": None, "humidity_pct": None}

    def _read_dht(self):
        if self._dht is None:
            return
        now = time.ticks_ms()
        if time.ticks_diff(now, self._last_dht_ms) < 2000:
            return  # DHT22 sampling limit
        try:
            self._dht.measure()
            self._cached["temp_c"] = self._dht.temperature()
            self._cached["humidity_pct"] = self._dht.humidity()
            self._last_dht_ms = now
        except OSError:
            pass

    def _soil_pct(self, raw):
        span = config.SOIL_DRY_RAW - config.SOIL_WET_RAW
        if span <= 0:
            return 0
        pct = (config.SOIL_DRY_RAW - raw) * 100 / span
        return max(0, min(100, int(pct)))

    def read_all(self):
        self._read_dht()
        soil = [self._soil_pct(a.read_u16() >> 4) for a in self._soil]
        light = self._light.read_u16() >> 4
        return {
            "temp_c": self._cached["temp_c"],
            "humidity_pct": self._cached["humidity_pct"],
            "soil_pct": soil,
            "light_raw": light,
            "ts_ms": time.ticks_ms(),
        }
