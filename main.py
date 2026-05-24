import time
import gc

import config
from lib import wifi
from lib.relays import RelayBank
from lib.sensors import SensorBank


def _try_import_display():
    try:
        from lib.display import Display
        from lib.touch import Touch
        from lib.ui import Dashboard
        return Display, Touch, Dashboard
    except ImportError as e:
        print("display unavailable:", e)
        return None, None, None


def _try_telemetry(relays):
    try:
        from lib.mqtt import Telemetry
        tel = Telemetry(relays)
        tel.connect()
        return tel
    except Exception as e:
        print("mqtt unavailable:", e)
        return None


def run():
    import lvgl as lv  # safe: only used when display loaded successfully

    sensors = SensorBank()
    relays = RelayBank()

    Display, Touch, Dashboard = _try_import_display()
    display = Display(rotation=0) if Display else None
    Touch() if Touch else None
    dash = Dashboard(relays) if (Dashboard and display) else None

    telemetry = _try_telemetry(relays)
    sta = wifi.connect()
    start_ms = time.ticks_ms()

    last_sensor_ms = 0
    while True:
        now = time.ticks_ms()
        relays.tick()

        if time.ticks_diff(now, last_sensor_ms) >= 1000:
            reading = sensors.read_all()
            if dash:
                dash.update_sensors(reading)
                dash.update_relays()
            if telemetry:
                try:
                    telemetry.maybe_publish(reading)
                except OSError as e:
                    print("mqtt publish error:", e)
            last_sensor_ms = now

        if telemetry:
            try:
                telemetry.poll()
            except OSError:
                pass

        if dash:
            dash.update_status(
                sta.isconnected(),
                bool(telemetry),
                wifi.ip(sta),
                time.ticks_diff(now, start_ms) // 1000,
            )
            lv.task_handler()

        time.sleep_ms(5)
        gc.collect() if (now & 0x3FF) == 0 else None


if __name__ == "__main__":
    try:
        run()
    except Exception as e:
        print("main crashed:", e)
        import sys
        sys.print_exception(e)
