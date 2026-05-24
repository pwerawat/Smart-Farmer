import sys

sys.path.append("/lib")

import gc
gc.collect()

try:
    from lib import wifi
    wifi.connect()
except Exception as e:
    print("boot: wifi connect failed:", e)
