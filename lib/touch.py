# AXS15231B capacitive touch driver (I2C).
# Reports a single touch point; the controller supports up to 5 but the
# UI doesn't need multi-touch.

from machine import I2C, Pin
from lib import board

try:
    import lvgl as lv
except ImportError:
    lv = None


_READ_CMD = bytes([0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08])


class Touch:
    def __init__(self, i2c=None):
        self.i2c = i2c or I2C(0, sda=Pin(board.TOUCH_SDA),
                              scl=Pin(board.TOUCH_SCL), freq=400_000)
        self.addr = board.TOUCH_ADDR
        self._buf = bytearray(8)
        self.last_x = 0
        self.last_y = 0
        self.pressed = False
        if lv is not None:
            self.indev = lv.indev_create()
            self.indev.set_type(lv.INDEV_TYPE.POINTER)
            self.indev.set_read_cb(self._lv_read)

    def read(self):
        try:
            self.i2c.writeto(self.addr, _READ_CMD)
            self.i2c.readfrom_into(self.addr, self._buf)
        except OSError:
            self.pressed = False
            return None
        b = self._buf
        finger = b[1]
        if finger == 0 or finger > 5:
            self.pressed = False
            return None
        x = ((b[2] & 0x0F) << 8) | b[3]
        y = ((b[4] & 0x0F) << 8) | b[5]
        self.last_x = x
        self.last_y = y
        self.pressed = True
        return (x, y)

    def _lv_read(self, indev, data):
        self.read()
        data.point.x = self.last_x
        data.point.y = self.last_y
        data.state = lv.INDEV_STATE.PRESSED if self.pressed else lv.INDEV_STATE.RELEASED
