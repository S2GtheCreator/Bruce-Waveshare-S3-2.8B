Bruce base: 1.16.1, ba519c936c87b89c9c667c15b932d7faca5360d5
https://github.com/BruceDevices/firmware/releases/tag/1.16.1
Waveshare demo: https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8B/ESP32-S3-Touch-LCD-2.8B-Demo.zip
Sources: Arduino/examples/LVGL_Arduino/{Display_ST7701.*,Touch_GT911.*,TCA9554PWR.*,SD_Card.*,I2C_Driver.*}
Arduino_GFX vendored commit: c2e39350ff09c3720702bd3f050cb581fa45a8e9
https://github.com/moononournation/Arduino_GFX
Driver changes: RGB aborts replaced with checked returns.
HAL: initialize recursive mutex, check begin, 480*10 pixel bounce buffers.
Single RGB565 framebuffer: 614400 bytes PSRAM; two bounce buffers: 19200 bytes internal SRAM.
No LVGL or duplicate UI. Original Bruce menus, CLI and tools retained.

Additional port changes: RGB constructor fallthrough fixed in existing HAL;
pinouts variant routing; original Bruce Wi-Fi archive patch uses unified Xtensa objcopy with checked result.
qio_opi SDK confirmed: CONFIG_SPIRAM_MODE_OCT=1, CONFIG_SPIRAM_SPEED_80M=1.
Battery percentage disabled (no verified calibration); physical hardware tests not performed.

BLE link diagnostics: target's raw NimBLE include now matches NimBLE-Arduino headers;
two external BLE helper callback class names made distinct by dependency patch script.
No BLE tool implementation or menu removed. Hardware BLE validation still required.

FastLED 3.10.6 unused IDF BLE JSON-RPC transport disabled for this board:
its IDF NimBLE structs differed from Bruce's NimBLE-Arduino ABI at link time.
Bruce BLE tools remain enabled. This avoids introducing a second BLE stack.
