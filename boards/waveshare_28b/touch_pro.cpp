#include <Arduino.h>
#if WS28B_TOUCH_PRO
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/scrollableTextArea.h"
#include <esp_heap_caps.h>
#include <freertos/task.h>
void waveshareTouchProTest() {
    drawMainBorder();
    printTitle("Touch / text check");
    tft.setTextSize(2);
    tft.drawString("çğıöşü ÇĞİÖŞÜ", 20, 80);
    tft.setTextSize(1);
    tft.drawString("Tap corners/center. Back/BOOT exits.", 20, 115);
    tft.drawRect(12, 150, tftWidth - 24, tftHeight - 210, bruceConfig.priColor);
    int oldX = -1, oldY = -1;
    while (!check(EscPress)) {
        int x = touchPoint.x, y = touchPoint.y;
        if (touchPoint.pressed && (x != oldX || y != oldY)) {
            tft.fillRect(20, 130, tftWidth - 40, 16, bruceConfig.bgColor);
            tft.drawString(
                "X=" + String(x) + " Y=" + String(y) + " LCD " + String(tft.width()) + "x" +
                    String(tft.height()),
                20,
                130
            );
            if (x >= 0 && x < tftWidth && y >= 150 && y < tftHeight - 60) tft.fillCircle(x, y, 3, TFT_GREEN);
            oldX = x;
            oldY = y;
        }
        delay(20);
    }
}
void waveshareResources() {
    String report = "Internal free heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    report += "Minimum free heap: " + String(ESP.getMinFreeHeap()) + " bytes\n";
    report += "Largest internal block: " +
              String(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)) + " bytes\n";
    report += "Largest DMA block: " + String(heap_caps_get_largest_free_block(MALLOC_CAP_DMA)) + " bytes\n";
    report += "PSRAM found: " + String(ESP.getPsramSize()) +
              " bytes\nPSRAM free: " + String(ESP.getFreePsram()) + " bytes\n";
    report += "UI task minimum free stack: " + String(uxTaskGetStackHighWaterMark(nullptr)) + " bytes\n";
    report += "RGB framebuffer: 614400 bytes PSRAM\nTwo bounce buffers: 19200 bytes internal SRAM\nNo second "
              "framebuffer, no LVGL buffers.\n";
    report += "Compare before/after 20 open/close cycles. Fragmentation, active WiFi/USB and filesystem "
              "caches also affect free heap.\n";
    ScrollableTextArea area("Memory snapshot");
    area.fromString(report);
    area.show();
}

String waveshareTextKeyboard(const String &original, size_t limit, const String &title, bool *cancelled) {
    if (cancelled) *cancelled = false;
    String value = original;
    for (;;) {
        std::vector<Option> menu = {
            {"Append keyboard / symbols",
             [&]() {
                 if (value.length() >= limit) return;
                 String next = keyboard("", limit - value.length(), title);
                 if (next != "\x1B") value += next;
             }                                                 },
            {"Turkish letters",
             [&]() {
                 const char *letters[] = {"ç", "ğ", "ı", "ö", "ş", "ü", "Ç", "Ğ", "İ", "Ö", "Ş", "Ü"};
                 std::vector<Option> keys;
                 for (const char *letter : letters) {
                     String k = letter;
                     keys.push_back({k, [&, k]() {
                                         if (value.length() + k.length() <= limit) value += k;
                                     }});
                 }
                 loopOptions(keys, MENU_TYPE_SUBMENU, "Insert Turkish letter");
             }                                                 },
            {"Backspace UTF-8",
             [&]() {
                 if (!value.length()) return;
                 size_t at = value.length() - 1;
                 while (at && (uint8_t(value[at]) & 0xC0) == 0x80) --at;
                 value.remove(at);
             }                                                 },
            {"Clear field",               [&]() { value = ""; }},
            {"Preview",
             [&]() {
                 ScrollableTextArea area(title);
                 area.fromString(value);
                 area.show();
             }                                                 },
            {"Done",                      []() {}              },
            {"Cancel",                    []() {}              }
        };
        int n = loopOptions(menu, MENU_TYPE_SUBMENU, title.c_str());
        if (n == 5) return value;
        if (n < 0 || n == 6) {
            if (cancelled) *cancelled = true;
            return original;
        }
    }
}
#endif
