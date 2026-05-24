# Pin map for the Guition JC3248W535 (ESP32-S3, 3.2" 320x480 QSPI, AXS15231B).
# Values match the manufacturer's published schematic; verify against your
# revision before powering external loads.

# Display (AXS15231B, QSPI)
LCD_CS   = 45
LCD_SCK  = 47
LCD_D0   = 21
LCD_D1   = 48
LCD_D2   = 40
LCD_D3   = 39
LCD_TE   = 38
LCD_RST  = -1   # tied to power rail on this board
LCD_BL   = 1

LCD_WIDTH  = 320
LCD_HEIGHT = 480
LCD_BPP    = 16

# Touch (AXS15231B touch controller, I2C)
TOUCH_SDA = 4
TOUCH_SCL = 8
TOUCH_INT = 3
TOUCH_RST = -1
TOUCH_ADDR = 0x3B

# microSD (shared SPI on the board's TF slot)
SD_CS   = 41
SD_SCK  = 39   # shared; only usable when display is idle
SD_MOSI = 38
SD_MISO = 40

# Onboard status LED (WS2812)
RGB_LED = 42

# USB (do not repurpose)
USB_DM = 19
USB_DP = 20
