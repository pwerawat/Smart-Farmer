import time

import network

import config


def connect(timeout_s=None):
    timeout_s = timeout_s or config.WIFI_TIMEOUT_S
    sta = network.WLAN(network.STA_IF)
    sta.active(True)
    if sta.isconnected():
        return sta
    sta.connect(config.WIFI_SSID, config.WIFI_PASSWORD)
    deadline = time.ticks_add(time.ticks_ms(), timeout_s * 1000)
    while not sta.isconnected():
        if time.ticks_diff(deadline, time.ticks_ms()) <= 0:
            return sta
        time.sleep_ms(200)
    return sta


def ip(sta=None):
    sta = sta or network.WLAN(network.STA_IF)
    if not sta.isconnected():
        return None
    return sta.ifconfig()[0]
