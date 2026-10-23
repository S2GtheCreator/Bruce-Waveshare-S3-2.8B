#include "pro_modules.h"
#include "pro_network_text.h"
#if WS28B_WIFI_ANALYZER_PRO
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/scrollableTextArea.h"
#include "core/sd_functions.h"
#include "modules/badusb_ble/ducky_typer.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <algorithm>
#include <array>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <memory>
#include <time.h>

namespace {
struct AP {
    String ssid, bssid;
    int32_t rssi = 0;
    uint8_t channel = 0, auth = 0;
    uint32_t seen = 0;
    bool hidden = false, compare = false;
};
struct Analyzer {
    std::array<AP, 64> aps;
    size_t count = 0;
    uint32_t sequence = 0, salt = esp_random();
    bool masked = true;
    wifi_mode_t before = WiFi.getMode();
    bool changed = false, ownsScan = false;
    ~Analyzer() {
        if (ownsScan) {
            esp_wifi_scan_stop();
            WiFi.scanDelete();
        }
        if (changed) WiFi.mode(before);
    }
    bool scan() {
        if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
            displayError("Another scan is active", true);
            return false;
        }
        if (esp_get_free_heap_size() < 70000) {
            displayError("Not enough internal heap", true);
            return false;
        }
        // Passive: no probe requests, association, monitor injection or disconnect.
        int n = WiFi.scanNetworks(true, true, true, 300);
        ownsScan = n == WIFI_SCAN_RUNNING;
        const uint32_t started = millis();
        while (n == WIFI_SCAN_RUNNING) {
            if (check(EscPress) || uint32_t(millis() - started) > 15000) {
                esp_wifi_scan_stop();
                WiFi.scanDelete();
                ownsScan = false;
                return false;
            }
            delay(20);
            n = WiFi.scanComplete();
        }
        if (n < 0) {
            displayError("Passive scan failed", true);
            if (ownsScan) WiFi.scanDelete();
            ownsScan = false;
            return false;
        }
        std::array<String, 64> selected;
        size_t selectedCount = 0;
        for (size_t i = 0; i < count; ++i)
            if (aps[i].compare) selected[selectedCount++] = aps[i].bssid;
        count = std::min(n, 64);
        for (size_t i = 0; i < count; ++i) {
            AP &a = aps[i];
            a = AP();
            a.ssid = WiFi.SSID(i);
            a.hidden = a.ssid.isEmpty();
            a.bssid = WiFi.BSSIDstr(i);
            a.channel = WiFi.channel(i);
            a.rssi = WiFi.RSSI(i);
            a.auth = uint8_t(WiFi.encryptionType(i));
            a.seen = millis();
            for (size_t k = 0; k < selectedCount; ++k)
                if (selected[k] == a.bssid) a.compare = true;
        }
        ++sequence;
        WiFi.scanDelete();
        ownsScan = false;
        return true;
    }
    String security(uint8_t a) const {
        switch (a) {
            case 0: return "OPEN";
            case 1: return "WEP";
            case 2: return "WPA";
            case 3: return "WPA2";
            case 4: return "WPA/WPA2";
            case 5: return "WPA2 enterprise";
            case 6: return "WPA3";
            case 7: return "WPA2/WPA3";
            default: return String("auth=") + a;
        }
    }
    String identity(const AP &a) const {
        if (!masked) return a.bssid;
        // Per-session salted pseudonym; no false claim of irreversible anonymization.
        uint32_t hash = 2166136261u ^ salt;
        for (size_t i = 0; i < a.bssid.length(); ++i) hash = (hash ^ uint8_t(a.bssid[i])) * 16777619u;
        char b[20];
        snprintf(b, sizeof(b), "AP-%08lx", (unsigned long)hash);
        return b;
    }
    void show(const String &title, const String &text) {
        ScrollableTextArea area(title);
        area.fromString(text);
        area.show();
    }
    void channels() {
        String text =
            "2.4 GHz passive AP observations\nCounts/RSSI are NOT spectrum occupancy or airtime %.\n";
        for (int c = 1; c <= 14; ++c) {
            int n = 0, strong = 0, medium = 0, weak = 0, nearby = 0;
            for (size_t i = 0; i < count; ++i) {
                const AP &a = aps[i];
                if (a.channel == c) {
                    ++n;
                    if (a.rssi >= -60) ++strong;
                    else if (a.rssi >= -75) ++medium;
                    else ++weak;
                }
                if (abs(int(a.channel) - c) <= 4) ++nearby;
            }
            text += "CH " + String(c) + ": " + String(n) + " AP [";
            for (int k = 0; k < std::min(n, 16); ++k) text += "#";
            text += "]\nRSSI strong/mid/weak " + String(strong) + "/" + String(medium) + "/" + String(weak) +
                    "\n";
            text += "Nearby centers (+/-4): " + String(nearby) + " (20MHz assumption, width unknown)\n";
        }
        show("Channel comparison", text);
    }
    void compare() {
        String text;
        for (size_t i = 0; i < count; ++i)
            if (aps[i].compare) {
                const AP &a = aps[i];
                text += wsNetworkLabel(a.ssid) + "\n" + a.bssid + " CH" + String(a.channel) + " " +
                        String(a.rssi) + "dBm " + security(a.auth) + "\nAge " +
                        String((millis() - a.seen) / 1000) + "s\n";
            }
        show("Selected APs", text.length() ? text : "Select APs in Network details first.");
    }
    void graph(const String &bssid, uint32_t window) {
        struct Sample {
            uint32_t at;
            int16_t rssi;
        };
        std::array<Sample, 61> samples{};
        size_t used = 0;
        uint32_t next = 0;
        while (!check(EscPress)) {
            if (int32_t(millis() - next) >= 0) {
                if (!scan()) return;
                int16_t r = -127;
                for (size_t i = 0; i < count; ++i)
                    if (aps[i].bssid == bssid) r = aps[i].rssi;
                if (used == samples.size()) {
                    std::move(samples.begin() + 1, samples.end(), samples.begin());
                    --used;
                }
                samples[used++] = {millis(), r};
                next = millis() + 5000;
                drawMainBorder();
                printTitle("RSSI / time");
                tft.setTextSize(1);
                tft.drawString(bssid, 12, 65);
                tft.drawString("-30 dBm", 12, 90);
                const int x0 = 20, y0 = 120, w = tftWidth - 40, h = tftHeight - 210;
                tft.drawRect(x0, y0, w, h, bruceConfig.priColor);
                int px = -1, py = -1;
                for (size_t i = 0; i < used; ++i) {
                    uint32_t age = millis() - samples[i].at;
                    if (age > window) continue;
                    if (samples[i].rssi == -127) {
                        px = -1;
                        continue;
                    }
                    int x = x0 + w - int(uint64_t(age) * w / window),
                        y = y0 + std::min(70, std::max(0, -30 - int(samples[i].rssi))) * h / 70;
                    if (px >= 0) tft.drawLine(px, py, x, y, TFT_GREEN);
                    else tft.fillCircle(x, y, 2, TFT_GREEN);
                    px = x;
                    py = y;
                }
                tft.drawString("-100 dBm; missing AP = gap", 12, y0 + h + 5);
                tft.drawString(String(window / 1000) + "s window; sample >=5s; Back exits", 12, y0 + h + 22);
            }
            delay(20);
        }
    }
    String csv(const String &value) { return wsCsvField(value); }
    void exportReport(bool json) {
        FS *fs = nullptr;
        if (!getFsStorage(fs) || !fs) return;
        String path = "/wifi-analysis-" + String(millis()) + (json ? ".json" : ".csv"), tmp = path + ".tmp";
        if (fs->exists(path) || fs->exists(tmp)) {
            displayError("Report name exists", true);
            return;
        }
        File file = fs->open(tmp, FILE_WRITE);
        if (!file) {
            displayError("Cannot create report", true);
            return;
        }
        bool ok = true;
        auto write = [&](const String &s) {
            if (file.write((const uint8_t *)s.c_str(), s.length()) != s.length()) ok = false;
        };
        const time_t epoch = time(nullptr);
        if (json) {
            JsonDocument doc;
            doc["passive"] = true;
            doc["band"] = "2.4GHz";
            doc["uptime_ms"] = millis();
            if (epoch > 1577836800) doc["epoch_seconds"] = (int64_t)epoch;
            else doc["epoch_seconds"] = nullptr;
            doc["captured_aps"] = count;
            doc["limit"] = 64;
            doc["masked"] = masked;
            doc["occupancy_percent"] = nullptr;
            for (size_t i = 0; i < count; ++i) {
                const AP &a = aps[i];
                JsonObject o = doc["aps"].add<JsonObject>();
                o["ssid"] = masked ? identity(a) : wsNetworkLabel(a.ssid);
                o["ssid_format"] = masked ? "pseudonym" : "utf8-with-byte-escapes";
                o["bssid"] = identity(a);
                o["rssi_dbm"] = a.rssi;
                o["channel"] = a.channel;
                o["security"] = security(a.auth);
                o["seen_uptime_ms"] = a.seen;
                o["hidden"] = a.hidden;
                o["bandwidth_mhz"] = nullptr;
            }
            const size_t expected = measureJson(doc);
            ok = !doc.overflowed() && serializeJson(doc, file) == expected && expected > 0;
        } else {
            write("uptime_ms,epoch_seconds,ssid,bssid,channel,rssi_dbm,security,hidden,last_seen_ms\n");
            for (size_t i = 0; i < count; ++i) {
                const AP &a = aps[i];
                write(
                    String(millis()) + "," + (epoch > 1577836800 ? String((long long)epoch) : String()) +
                    "," + csv(masked ? identity(a) : a.ssid) + "," + csv(identity(a)) + "," +
                    String(a.channel) + "," + String(a.rssi) + "," + csv(security(a.auth)) + "," +
                    String(a.hidden) + "," + String(a.seen) + "\n"
                );
            }
        }
        file.flush();
        const size_t bytes = file.size();
        file.close();
        File check = fs->open(tmp, FILE_READ);
        ok = ok && check && check.size() == bytes && bytes > 0;
        check.close();
        if (ok) ok = fs->rename(tmp, path);
        if (!ok) {
            fs->remove(tmp);
            displayError("Report write/full/rename failed", true);
        } else displaySuccess(path, true);
    }
    void detail(size_t i) {
        String id = aps[i].bssid;
        std::vector<Option> menu = {
            {"Details",
             [&,                      id]() {
                 for (size_t j = 0; j < count; ++j)
                     if (aps[j].bssid == id) {
                         const AP &a = aps[j];
                         show(
                             "AP details",
                             wsNetworkLabel(a.ssid) + "\n" + a.bssid + "\nCH " + String(a.channel) + " / " +
                                 String(a.rssi) + "dBm\n" + security(a.auth) + "\nHidden: " +
                                 String(a.hidden) + "\nLast seen age " + String((millis() - a.seen) / 1000) +
                                 "s\nWidth / standards: not exposed by this scan API"
                         );
                     }
             }                                    },
            {"Toggle comparison",
             [&,                      id]() {
                 for (size_t j = 0; j < count; ++j)
                     if (aps[j].bssid == id) aps[j].compare = !aps[j].compare;
             }                                    },
            {"RSSI 30 seconds",   [&, id]() { graph(id, 30000); } },
            {"RSSI 60 seconds",   [&, id]() { graph(id, 60000); } },
            {"RSSI 5 minutes",    [&, id]() { graph(id, 300000); }}
        };
        loopOptions(menu, MENU_TYPE_SUBMENU, "Network");
    }
    void networks() {
        std::vector<Option> menu;
        for (size_t i = 0; i < count; ++i) {
            String id = aps[i].bssid;
            menu.push_back(
                {(aps[i].hidden ? String("[hidden]") : wsNetworkLabel(aps[i].ssid)) + " " +
                     String(aps[i].rssi),
                 [&, id]() {
                     for (size_t j = 0; j < count; ++j)
                         if (aps[j].bssid == id) {
                             detail(j);
                             return;
                         }
                     displayInfo("AP no longer in latest scan", true);
                 }}
            );
        }
        if (menu.empty()) {
            show("Networks", "No AP observed in the last passive scan.");
            return;
        }
        loopOptions(menu, MENU_TYPE_SUBMENU, "Networks (max 64)");
    }
    void run() {
        if ((before & WIFI_MODE_AP) || isWebUIActive) {
            displayError("Stop AP/WebUI before analysis", true);
            return;
        }
        if (activeBLEInstances > 0) {
            displayError("Stop BLE HID before analysis", true);
            return;
        }
        if (before == WIFI_MODE_NULL) {
            if (!WiFi.mode(WIFI_STA)) {
                displayError("STA start failed", true);
                return;
            }
            changed = true;
        }
        if (!scan()) return;
        for (;;) {
            std::vector<Option> menu = {
                {"Networks",                                        [&]() { networks(); }         },
                {"Rescan (passive)",                                [&]() { scan(); }             },
                {"Channel analysis",                                [&]() { channels(); }         },
                {"Compare selected",                                [&]() { compare(); }          },
                {String("Mask export: ") + (masked ? "ON" : "OFF"), [&]() { masked = !masked; }   },
                {"Export CSV",                                      [&]() { exportReport(false); }},
                {"Export JSON",                                     [&]() { exportReport(true); } },
                {"Back",                                            []() {}                       }
            };
            int n = loopOptions(menu, MENU_TYPE_SUBMENU, "Wi-Fi Analyzer Pro");
            if (n < 0 || n == int(menu.size()) - 1) break;
        }
    }
};
} // namespace
void waveshareAnalyzerPro() {
    std::unique_ptr<Analyzer> a(new (std::nothrow) Analyzer);
    if (!a) {
        displayError("Analyzer allocation failed", true);
        return;
    }
    a->run();
}
#endif
