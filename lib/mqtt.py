import json
import time

import config

try:
    from umqtt.simple import MQTTClient
except ImportError:
    MQTTClient = None


class Telemetry:
    def __init__(self, relays, on_command=None):
        if MQTTClient is None:
            raise ImportError("umqtt.simple missing -- install via mip or freeze it")
        self.relays = relays
        self.on_command = on_command
        self.client = MQTTClient(
            config.MQTT_CLIENT_ID,
            config.MQTT_BROKER,
            port=config.MQTT_PORT,
            user=config.MQTT_USER,
            password=config.MQTT_PASSWORD,
            keepalive=60,
        )
        self.client.set_callback(self._on_msg)
        self._last_pub_ms = 0
        self._connected = False

    def connect(self):
        self.client.connect()
        self.client.subscribe(config.MQTT_TOPIC_COMMAND)
        self._connected = True

    def _on_msg(self, topic, payload):
        # Commands: smartfarmer/cmd/<label>  payload = {"on": true} or {"pulse_s": 30}
        try:
            label = topic.decode().rsplit("/", 1)[-1]
            data = json.loads(payload)
        except (ValueError, UnicodeError):
            return
        relay = self.relays.get(label)
        if not relay:
            return
        if "pulse_s" in data:
            relay.pulse(float(data["pulse_s"]))
        elif "on" in data:
            relay.set(bool(data["on"]))
        if self.on_command:
            self.on_command(label, data)

    def maybe_publish(self, reading):
        now = time.ticks_ms()
        if time.ticks_diff(now, self._last_pub_ms) < config.MQTT_PUBLISH_INTERVAL_S * 1000:
            return
        payload = dict(reading)
        payload["relays"] = {r.label: r.is_on() for r in self.relays}
        self.client.publish(config.MQTT_TOPIC_TELEMETRY, json.dumps(payload))
        self._last_pub_ms = now

    def poll(self):
        if self._connected:
            self.client.check_msg()
