# User configuration. Copy to config_local.py and edit, or edit in place.
# Loaded by boot.py and main.py.

WIFI_SSID = "your-ssid"
WIFI_PASSWORD = "your-password"
WIFI_TIMEOUT_S = 20

MQTT_BROKER = "192.168.1.10"
MQTT_PORT = 1883
MQTT_USER = None
MQTT_PASSWORD = None
MQTT_CLIENT_ID = "smart-farmer-01"
MQTT_TOPIC_TELEMETRY = "smartfarmer/telemetry"
MQTT_TOPIC_COMMAND = "smartfarmer/cmd/#"
MQTT_PUBLISH_INTERVAL_S = 30

# Relays: list of (label, gpio, active_high)
RELAYS = [
    ("Pump 1", 10, True),
    ("Pump 2", 11, True),
    ("Valve A", 12, True),
    ("Valve B", 13, True),
]

# Sensors. Pins are placeholders -- wire to free GPIOs on the board's
# expansion header and update here.
DHT_PIN = 17           # DHT22 data pin
SOIL_ADC_PINS = [18]   # capacitive soil moisture sensors (ADC1 channels)
LIGHT_ADC_PIN = 14     # LDR or analog light sensor

# Calibration for capacitive soil moisture (raw ADC -> %)
SOIL_DRY_RAW = 3000    # reading in air
SOIL_WET_RAW = 1200    # reading in water

SCREEN_DIM_AFTER_S = 60
