from machine import Pin
import time

import config


class Relay:
    def __init__(self, label, gpio, active_high=True):
        self.label = label
        self.active_high = active_high
        self.pin = Pin(gpio, Pin.OUT, value=0 if active_high else 1)
        self._on = False
        self._until_ms = None

    def set(self, on):
        self._on = bool(on)
        self.pin.value(1 if (self._on == self.active_high) else 0)
        self._until_ms = None

    def pulse(self, duration_s):
        self.set(True)
        self._until_ms = time.ticks_add(time.ticks_ms(), int(duration_s * 1000))

    def is_on(self):
        return self._on

    def tick(self):
        if self._until_ms is not None and time.ticks_diff(self._until_ms, time.ticks_ms()) <= 0:
            self.set(False)


class RelayBank:
    def __init__(self):
        self.relays = [Relay(l, g, a) for (l, g, a) in config.RELAYS]
        self._by_label = {r.label: r for r in self.relays}

    def __iter__(self):
        return iter(self.relays)

    def get(self, label):
        return self._by_label.get(label)

    def tick(self):
        for r in self.relays:
            r.tick()

    def all_off(self):
        for r in self.relays:
            r.set(False)
