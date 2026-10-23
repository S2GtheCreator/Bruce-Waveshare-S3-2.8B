#pragma once
#define TOUCH_FOOTER_HEIGHT 32
#include <stdint.h>
#include "soc/soc_caps.h"
#include "sdkconfig.h"
// Fail closed: this PCB has no external CH9329 keyboard controller.
#if !defined(USB_as_HID) || !USB_as_HID
#error "waveshare_28b requires USB_as_HID=1 (native ESP32-S3 USB keyboard)"
#endif
#if !SOC_USB_OTG_SUPPORTED || !CONFIG_TINYUSB_ENABLED || !CONFIG_TINYUSB_HID_ENABLED
#error "waveshare_28b requires USB OTG and TinyUSB HID support in the SDK"
#endif
#if !defined(ARDUINO_USB_MODE) || ARDUINO_USB_MODE != 0
#error "waveshare_28b requires ARDUINO_USB_MODE=0 (USB OTG/TinyUSB)"
#endif
#if !ARDUINO_USB_CDC_ON_BOOT || !CONFIG_TINYUSB_CDC_ENABLED
#error "waveshare_28b requires TinyUSB CDC for the existing serial CLI"
#endif
#if defined(LITE_VERSION)
#error "waveshare_28b native HID requires the full Bruce Bad_Usb_Lib"
#endif
#define DEVICE_NAME "Waveshare 2.8B"
#define HAS_SCREEN 1
#define HAS_TOUCH 1
#define HAS_CAPACITIVE_TOUCH 1
#define USE_ARDUINO_GFX 1
#define RGB_PANEL 1
#define TFT_DATABUS_N 3
#define TFT_DISPLAY_DRIVER_N 49
#define TFT_WIDTH 480
#define TFT_HEIGHT 640
#define ROTATION 0
#define TFT_DE 40
#define TFT_VSYNC 39
#define TFT_HSYNC 38
#define TFT_PCLK 41
// RGB565 data bits 0..15, official Display_ST7701.h
#define TFT_B0 5
#define TFT_B1 45
#define TFT_B2 48
#define TFT_B3 47
#define TFT_B4 21
#define TFT_G0 14
#define TFT_G1 13
#define TFT_G2 12
#define TFT_G3 11
#define TFT_G4 10
#define TFT_G5 9
#define TFT_R0 46
#define TFT_R1 3
#define TFT_R2 8
#define TFT_R3 18
#define TFT_R4 17
#define TFT_HSYNC_POL 1
#define TFT_HSYNC_FRONT_PORCH 60
#define TFT_HSYNC_PULSE_WIDTH 10
#define TFT_HSYNC_BACK_PORCH 70
#define TFT_VSYNC_POL 1
#define TFT_VSYNC_FRONT_PORCH 20
#define TFT_VSYNC_PULSE_WIDTH 10
#define TFT_VSYNC_BACK_PORCH 20
#define TFT_PCLK_ACTIVE_NEG 0
#define TFT_PREF_SPEED 30000000
#define BACKLIGHT 6
#define TFT_BL 6
#define MINBRIGHT 1
#define SYS_I2C_SDA 15
#define SYS_I2C_SCL 7
#define GROVE_SDA 15
#define GROVE_SCL 7
#define USE_SD_MMC 1
#define BOARD_MAX_SDMMC_FREQ 20000
#define SPI_SCK_PIN -1
#define SPI_MISO_PIN -1
#define SPI_MOSI_PIN -1
#define SPI_SS_PIN -1
#define TXLED -1
#define RXLED -1
#define RGB_LED -1
#define SERIAL_TX 43
#define SERIAL_RX 44
#define GPS_SERIAL_TX 43
#define GPS_SERIAL_RX 44
#define BAD_TX 43
#define BAD_RX 44
#define BTN_ALIAS "BOOT"
#define BTN_PIN 0
#define HAS_BTN 1
static const uint8_t TX=43, RX=44, SDA=15, SCL=7;
// No generic SPI bus is wired on this board. SD uses SDMMC.
static const int8_t SS=-1, MOSI=-1, MISO=-1, SCK=-1;
