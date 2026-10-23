#include "pro_modules.h"
#include "pro_script_limits.h"
#if WS28B_BADUSB_PRO
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/scrollableTextArea.h"
#include "core/sd_functions.h"
#include "modules/badusb_ble/ducky_typer.h"
#include "pro_modules.h"
#include <ArduinoJson.h>
#include <algorithm>
#include <atomic>
#include <time.h>
#include <tusb.h>

namespace {
constexpr size_t MAX_SCRIPT = WS_SCRIPT_MAX_BYTES, MAX_LINE = WS_SCRIPT_MAX_LINE_BYTES, MAX_RECORDS = 24;
std::atomic<bool> running{false};
bool aborted = false;
const String *approvedDocument = nullptr;
uint32_t lastProgress = 0;
const char *INDEX = "/.ws28b-pro-index.json";
bool confirmation(const String &title) {
    bool yes = false;
    std::vector<Option> m = {
        {"Cancel",  []() {}              },
        {"Confirm", [&]() { yes = true; }}
    };
    loopOptions(m, MENU_TYPE_SUBMENU, title.c_str());
    return yes;
}
void view(const String &title, const String &text) {
    ScrollableTextArea a(title);
    a.fromString(text);
    a.show();
}
String textKeyboard(const String &original, size_t limit, const String &title) {
#if WS28B_TOUCH_PRO
    return waveshareTextKeyboard(original, limit, title);
#else
    String value = original;
    for (;;) {
        std::vector<Option> menu = {
            {"Keyboard / symbols",
             [&]() {
                 if (value.length() >= limit) return;
                 String next = keyboard("", limit - value.length(), title);
                 if (next != "\x1B") value += next;
             }                                                  },
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
             }                                                  },
            {"Backspace UTF-8",
             [&]() {
                 if (!value.length()) return;
                 size_t at = value.length() - 1;
                 while (at && (uint8_t(value[at]) & 0xC0) == 0x80) --at;
                 value.remove(at);
             }                                                  },
            {"Clear field",        [&]() { value = ""; }        },
            {"Preview",            [&]() { view(title, value); }},
            {"Done",               []() {}                      },
            {"Cancel",             []() {}                      }
        };
        int n = loopOptions(menu, MENU_TYPE_SUBMENU, title.c_str());
        if (n == 5) return value;
        if (n < 0 || n == 6) return original;
    }
#endif
}
bool readText(FS &fs, const String &path, String &out) {
    File f = fs.open(path, FILE_READ);
    if (!f || f.isDirectory() || f.size() > MAX_SCRIPT) {
        f.close();
        displayError("File unavailable or >16 KiB", true);
        return false;
    }
    const size_t bytes = f.size();
    out = "";
    if (!out.reserve(bytes + 1)) {
        f.close();
        displayError("Text allocation failed", true);
        return false;
    }
    const uint32_t started = millis();
    for (size_t i = 0; i < bytes; ++i) {
        if ((i & 63) == 0) {
            if (check(EscPress) || uint32_t(millis() - started) > 5000) {
                f.close();
                displayError("Script read cancelled / timeout", true);
                return false;
            }
            yield();
        }
        const int c = f.read();
        if (c < 0) {
            f.close();
            displayError("Script read failed", true);
            return false;
        }
        out += char(c);
    }
    f.close();
    if (out.length() != bytes) {
        displayError("File read incomplete", true);
        return false;
    }
    size_t badLine = 0;
    if (wsScriptValidate(out.c_str(), out.length(), badLine) != WsScriptLimit::None) {
        displayError("Pro limit: 128 lines / 512 bytes per line; line " + String(badLine), true);
        return false;
    }
    return true;
}
bool safeSave(FS &fs, const String &path, const String &data) {
    const String tmp = path + ".pro-tmp", bak = path + ".pro-bak";
    if (fs.exists(tmp) || fs.exists(bak)) {
        displayError("Recovery file exists; inspect .pro-tmp/.pro-bak", true);
        return false;
    }
    File f = fs.open(tmp, FILE_WRITE);
    if (!f) {
        displayError("Create failed", true);
        return false;
    }
    bool ok = f.write((const uint8_t *)data.c_str(), data.length()) == data.length();
    f.flush();
    f.close();
    File verify = fs.open(tmp, FILE_READ);
    ok = ok && verify && verify.size() == data.length();
    for (size_t i = 0; ok && i < data.length(); ++i) ok = verify.read() == uint8_t(data[i]);
    verify.close();
    if (!ok) {
        fs.remove(tmp);
        displayError("Write/verify failed (full filesystem?)", true);
        return false;
    }
    const bool old = fs.exists(path);
    if (old && !fs.rename(path, bak)) {
        fs.remove(tmp);
        displayError("Backup rename failed", true);
        return false;
    }
    if (!fs.rename(tmp, path)) {
        if (old) fs.rename(bak, path);
        displayError("Commit failed; recovery file retained", true);
        return false;
    }
    if (old && !fs.remove(bak)) { displayWarning("Saved; old .pro-bak remains", true); }
    return true;
}
struct Manager {
    FS &fs;
    JsonDocument index;
    explicit Manager(FS &f) : fs(f) {
        File in = fs.open(INDEX, FILE_READ);
        if (in && in.size() <= 8192) {
            auto e = deserializeJson(index, in);
            if (e) index.clear();
        }
        in.close();
        if (index["scripts"].size() > MAX_RECORDS) index.clear();
    }
    void saveIndex() {
        String json;
        serializeJson(index, json);
        if (index.overflowed() || json.length() > 8192) {
            displayError("Metadata allocation/8 KiB limit", true);
            return;
        }
        safeSave(fs, INDEX, json);
    }
    void movedMetadata(const String &from, const String &to) {
        JsonObject r = index["scripts"].as<JsonObject>();
        if (!r[from].isNull()) {
            r[to].set(r[from]);
            r.remove(from);
            saveIndex();
        }
    }
    void metadata(const String &path, const char *key, const String &value) {
        JsonObject records = index["scripts"].as<JsonObject>();
        if (records.isNull()) records = index["scripts"].to<JsonObject>();
        if (records.size() >= MAX_RECORDS && records[path].isNull()) {
            displayError("Metadata limit 24 scripts", true);
            return;
        }
        records[path][key] = value;
        if (String(key) == "recent") {
            uint32_t seq = index["sequence"] | 0u;
            index["sequence"] = ++seq;
            records[path]["order"] = seq;
        }
        saveIndex();
    }
    String newPath(const String &initial) {
        String name = textKeyboard(initial, 64, "Script name (.txt)");
        if (name.isEmpty()) return "";
        name.trim();
        if (name == "\x1B") return "";
        for (size_t i = 0; i < name.length(); ++i)
            if (uint8_t(name[i]) < 32) {
                displayError("Control byte in filename", true);
                return "";
            }
        if (!name.length() || name.indexOf('/') >= 0 || name.indexOf('\\') >= 0 || name.indexOf("..") >= 0 ||
            name[0] == '.') {
            displayError("Use a plain filename", true);
            return "";
        }
        if (!name.endsWith(".txt")) name += ".txt";
        return "/" + name;
    }
    std::vector<String> split(const String &text) {
        std::vector<String> out;
        size_t start = 0;
        for (size_t i = 0; i < text.length(); ++i)
            if (text[i] == '\n') {
                String line = text.substring(start, i);
                if (line.endsWith("\r")) line.remove(line.length() - 1);
                out.push_back(line);
                start = i + 1;
            }
        if (start < text.length() || out.empty()) out.push_back(text.substring(start));
        return out;
    }
    String join(const std::vector<String> &lines) {
        String s;
        for (const auto &l : lines) {
            s += l;
            s += '\n';
        }
        return s;
    }
    void editor(String path, bool fresh = false) {
        if (ESP.getFreeHeap() < 96000) {
            displayError("Editor needs 96 KiB free heap", true);
            return;
        }
        String doc;
        if (!fresh && !readText(fs, path, doc)) return;
        String undo = doc;
        bool dirty = false;
        auto lines = split(doc);
        if (lines.size() > 128) {
            displayError("Editor limit: 128 lines", true);
            return;
        }
        for (const auto &l : lines)
            if (l.length() > MAX_LINE) {
                displayError("Editor limit: 512 bytes/line", true);
                return;
            }
        lines.clear();
        lines.shrink_to_fit();
        for (;;) {
            std::vector<Option> menu = {
                {"View document",               [&]() { view(path, doc); }},
                {"Edit / insert / delete line",
                 [&]() {
                     auto current = split(doc);
                     std::vector<Option> list;
                     for (size_t i = 0; i < current.size(); ++i)
                         list.push_back({String(i + 1) + ": " + current[i], [&, i]() {
                                             std::vector<Option> actions = {
                                                 {                                               "Edit",
                 [&,                              i]() {
                                                      String v = textKeyboard(
                                                          current[i], MAX_LINE, "Line " + String(i + 1)
                                                      );
                                                      undo = doc;
                                                      current[i] = v;
                                                      doc = join(current);
                                                      dirty = true;
                                                  }},
                                                 {                 "Insert before",
                 [&,       i]() {
                                                      if (current.size() >= 128) {
                                                          displayError("128 line limit", true);
                                                          return;
                                                      }
                                                      String v = textKeyboard("", MAX_LINE, "New line");
                                                      undo = doc;
                                                      current.insert(current.begin() + i, v);
                                                      doc = join(current);
                                                      dirty = true;
                                                  }},
                                                 {"Delete line", [&, i]() {
                                                      if (!confirmation("Delete this line?")) return;
                                                      undo = doc;
                                                      current.erase(current.begin() + i);
                                                      doc = join(current);
                                                      dirty = true;
                                                  }}
                                             };
                                             loopOptions(actions, MENU_TYPE_SUBMENU, "Line actions");
                                         }});
                     loopOptions(list, MENU_TYPE_SUBMENU, "Lines");
                 }                                                                          },
                {"Append line",
                 [&]() {
                     if (split(doc).size() >= 128) {
                         displayError("128 line limit", true);
                         return;
                     }
                     String v = textKeyboard("", MAX_LINE, "Append DuckyScript line");
                     undo = doc;
                     if (doc.length() && !doc.endsWith("\n")) doc += '\n';
                     doc += v + "\n";
                     dirty = true;
                 }                                                                         },
                {"Insert benign template",
                 [&]() {
                     undo = doc;
                     doc += "REM Text test\nDELAY 2000\nSTRING TEST\nENTER\n";
                     dirty = true;
                 }                                                                         },
                {"Undo / redo last edit",
                 [&]() {
                     String swap = doc;
                     doc = undo;
                     undo = swap;
                     dirty = true;
                 }                                      },
                {"Save",
                 [&]() {
                     if (doc.length() > MAX_SCRIPT) {
                         displayError("16 KiB limit", true);
                         return;
                     }
                     if (safeSave(fs, path, doc)) dirty = false;
                 }                                                        },
                {"Save as",
                 [&]() {
                     String dest = newPath("");
                     if (dest.length() && (!fs.exists(dest) || confirmation("Overwrite file?")) &&
                         doc.length() <= MAX_SCRIPT && safeSave(fs, dest, doc)) {
                         path = dest;
                         dirty = false;
                     }
                 }                                                        },
                {"Back", []() {}                                                        }
            };
            int n = loopOptions(menu, MENU_TYPE_SUBMENU, "Script editor");
            if (doc.length() > MAX_SCRIPT || split(doc).size() > 128) {
                doc = undo;
                displayError("16 KiB / 128 line limit; edit reverted", true);
            }
            if (n < 0 || n == int(menu.size()) - 1) {
                if (!dirty || confirmation("Discard unsaved changes?")) return;
            }
        }
    }
    void preview(const String &path) {
        if (ESP.getFreeHeap() < 70000) {
            displayError("Preview needs 70 KiB free heap", true);
            return;
        }

        String doc;
        if (!readText(fs, path, doc)) return;
        auto lines = split(doc);
        uint64_t delayMs = 0, lastWait = 0;
        const uint16_t previewKeyDelay = bruceConfig.badUSBBLEKeyDelay;
        uint32_t defaultTextDelay = previewKeyDelay;
        int nextTextDelay = -1;
        bool unboundedWait = false;
        String issues;
        size_t unsupported = 0;
        const int previewLayout = bruceConfig.badUSBBLEKeyboardLayout;
        const bool tr = previewLayout == 13;
        const uint8_t *layout = tr ? KeyboardLayout_tr_TR : KeyboardLayout_en_US;
        bool havePrevious = false;
        if (previewKeyDelay > 250) {
            issues += "Pro requires key delay <=250 ms for responsive Stop. Change BadUSB Settings or use "
                      "the original runner.\n";
            ++unsupported;
        }
        for (size_t i = 0; i < lines.size(); ++i) {
            String line = lines[i];
            if (line.endsWith("\r")) line.remove(line.length() - 1);
            int sp = line.indexOf(' ');
            String cmd = sp < 0 ? line : line.substring(0, sp),
                   arg = sp < 0 ? String() : line.substring(sp + 1);
            auto issue = [&](const String &message) {
                if (issues.length() < 2048) issues += "Line " + String(i + 1) + ": " + message + "\n";
                ++unsupported;
            };
            const bool stringDelay = cmd == "STRINGDELAY" || cmd == "STRING_DELAY" ||
                                     cmd == "DEFAULTSTRINGDELAY" || cmd == "DEFAULT_STRING_DELAY";
            if (cmd.length() && cmd != "REPEAT" && cmd != "DELAY") lastWait = 0;
            if (cmd == "DEFAULTDELAY" || cmd == "DEFAULT_DELAY") {
                lastWait = 100;
                delayMs += 100;
            }
            if (cmd == "WAIT_FOR_BUTTON_PRESS") unboundedWait = true;
            if (cmd == "DELAY" || stringDelay || cmd == "REPEAT") {
                bool digits = arg.length() > 0;
                for (size_t k = 0; k < arg.length(); ++k)
                    if (arg[k] < '0' || arg[k] > '9') digits = false;
                char *end = nullptr;
                unsigned long v = strtoul(arg.c_str(), &end, 10);
                bool valid = digits && end && *end == 0 && arg.length() <= 10 && v <= 0x7FFFFFFFUL;
                if (cmd == "REPEAT" && arg.isEmpty()) {
                    valid = true;
                    v = 1;
                }
                if (!valid || (cmd == "DELAY" && v == 0) || (cmd == "REPEAT" && (v == 0 || v > 65535)))
                    issue("Invalid numeric argument");
                else if (cmd == "DELAY") delayMs += v;
                else if (stringDelay && v > 250)
                    issue("Pro text delay limit 250 ms; original runner supports longer delays");
                if (valid) {
                    if (cmd == "REPEAT") delayMs += lastWait * v;
                    else if (cmd == "DELAY") lastWait = v;
                    else if (stringDelay) {
                        if (cmd.startsWith("DEFAULT")) defaultTextDelay = v;
                        else nextTextDelay = v;
                    }
                }
                if (cmd == "REPEAT" && !havePrevious) issue("REPEAT has no previous command");
            }
            if (cmd.length() && cmd != "REPEAT") havePrevious = true;
            if (cmd != "STRING" && cmd != "STRINGLN" && cmd != "REM" && cmd != "//")
                for (size_t k = 0; k < arg.length(); ++k)
                    if (uint8_t(arg[k]) >= 128) {
                        issue("Use STRING for Unicode; shortcut arguments must be ASCII");
                        break;
                    }
            if (cmd == "ALTCHAR" || cmd == "ALTSTRING" || cmd == "ALTCODE") {
                if (issues.length() < 2048)
                    issues += "Line " + String(i + 1) +
                              ": use STRING instead of ALT numeric codes in Pro (bounded Stop)\n";
                ++unsupported;
            }
            if (cmd == "STRING" || cmd == "STRINGLN") {
                HidTextError error;
                size_t strokes = 0;
                hidTextWrite(
                    (const uint8_t *)arg.c_str(),
                    arg.length(),
                    layout,
                    tr,
                    false,
                    [&](const HidTextStroke &) {
                        ++strokes;
                        return true;
                    },
                    error
                );
                if (error.failure != HidTextFailure::None) {
                    char b[96];
                    hidTextDescribe(error, b, sizeof(b));
                    if (issues.length() < 2048) issues += "Line " + String(i + 1) + ": " + b + "\n";
                    ++unsupported;
                }
            }
            if (cmd == "STRING" || cmd == "STRINGLN") {
                size_t strokes = 0;
                HidTextError dryError;
                hidTextWrite(
                    (const uint8_t *)arg.c_str(),
                    arg.length(),
                    layout,
                    tr,
                    false,
                    [&](const HidTextStroke &) {
                        ++strokes;
                        return true;
                    },
                    dryError
                );
                if (cmd == "STRINGLN") ++strokes;
                delayMs += uint64_t(strokes) * 2 * (nextTextDelay >= 0 ? nextTextDelay : defaultTextDelay);
                nextTextDelay = -1;
                lastWait = uint64_t(strokes) * 2 * defaultTextDelay;
            }
            if (cmd.length() && cmd != "REPEAT" && !findDuckyCommand(cmd.c_str()) &&
                !findDuckyCombination(cmd.c_str())) {
                if (issues.length() < 2048) issues += "Line " + String(i + 1) + ": unknown " + cmd + "\n";
                ++unsupported;
            }
        }
        const size_t lineCount = lines.size();
        lines.clear();
        lines.shrink_to_fit();
        String info =
            path + "\nLayout: " +
            String(
                tr                                         ? "Turkish Q"
                : bruceConfig.badUSBBLEKeyboardLayout == 0 ? "English US"
                                                           : "Other (preview limited)"
            ) +
            "\nUSB HID: " + String(tud_mounted() && !tud_suspended() ? "connected" : "not connected") +
            "\nLines: " + String(lineCount) +
            "\nNominal delays / typing: " + String((unsigned long long)delayMs) +
            " ms\nIncludes REPEAT, dead-key completion and configured text delays. USB/OS scheduling adds "
            "time.\n" +
            String(unboundedWait ? "WAIT_FOR_BUTTON_PRESS adds unbounded user time.\n" : "") + issues;
        view("Script preview", info);
        for (;;) {
            std::vector<Option> menu = {
                {"Read script",                [&]() { view(path, doc); }},
                {"Start with device approval",
                 [&]() {
                     if (unsupported) {
                         displayError("Fix unsupported text/commands first", true);
                         return;
                     }
                     if (bruceConfig.badUSBBLEKeyboardLayout != 0 && !tr) {
                         displayError("Pro preview supports TR Q / US", true);
                         return;
                     }
                     if (!confirmation("Send keys to connected computer?")) return;
                     if (bruceConfig.badUSBBLEKeyboardLayout != previewLayout ||
                         bruceConfig.badUSBBLEKeyDelay != previewKeyDelay) {
                         displayError("Keyboard settings changed; reopen preview", true);
                         return;
                     }
                     if (hid_usb || activeBLEInstances > 0) {
                         displayError("Close the current HID session first", true);
                         return;
                     }
                     if (running.exchange(true)) {
                         displayError("Pro runner already active", true);
                         return;
                     }
                     struct SessionGuard {
                         ~SessionGuard() {
                             if (running.load()) {
                                 approvedDocument = nullptr;
                                 if (hid_usb) ducky_endUsb(hid_usb);
                                 running = false;
                             }
                         }
                     } sessionGuard;
                     // Execute a verified snapshot so editing the source after preview
                     // cannot silently change the approved text.
                     const String snapshot = path + ".pro-run";
                     if (fs.exists(snapshot)) {
                         displayError("Old .pro-run exists; inspect it first", true);
                         return;
                     }
                     if (!safeSave(fs, snapshot, doc)) return;
                     returnToMenu = false;
                     ducky_startKb(hid_usb, false, 2);
                     if (!hid_usb || returnToMenu) {
                         fs.remove(snapshot);
                         return;
                     }
                     if (bruceConfig.badUSBBLEKeyboardLayout != previewLayout ||
                         bruceConfig.badUSBBLEKeyDelay != previewKeyDelay) {
                         fs.remove(snapshot);
                         displayError("Keyboard settings changed; reopen preview", true);
                         return;
                     }
                     hid_usb->setDelay(previewKeyDelay);
                     aborted = false;
                     lastProgress = 0;
                     approvedDocument = &doc;
                     key_input(fs, snapshot, hid_usb);
                     approvedDocument = nullptr;
                     hid_usb->releaseAll();
                     ducky_endUsb(hid_usb);
                     running = false;
                     if (!fs.remove(snapshot)) displayWarning("Run copy remains as .pro-run", true);
                     returnToMenu = false;
                     metadata(path, "recent", String((unsigned long)time(nullptr)));
                     metadata(path, "recent_uptime", String(millis()));
                     displayInfo(
                         aborted ? "Stopped; all HID keys released"
                                 : "Runner returned (see any errors above)",
                         true
                     );
                 }                                                       },
                {"Back",                       []() {}                   }
            };
            int n = loopOptions(menu, MENU_TYPE_SUBMENU, "Preview / run");
            if (n < 0 || n == int(menu.size()) - 1) return;
        }
    }
    void actions(String path) {
        for (;;) {
            if (!fs.exists(path) && fs.exists(path + ".pro-bak")) {
                if (confirmation("Restore interrupted save backup?")) fs.rename(path + ".pro-bak", path);
            }
            std::vector<Option> menu = {
                {"Preview / run",   [&]() { preview(path); }},
                {"View content",
                 [&]() {
                     String s;
                     if (readText(fs, path, s)) view(path, s);
                 }                                          },
                {"Edit",            [&]() { editor(path); } },
                {"Rename",
                 [&]() {
                     String to = newPath("");
                     if (to.length()) {
                         if (fs.exists(to)) {
                             displayError("Destination exists", true);
                             return;
                         }
                         if (fs.rename(path, to)) {
                             movedMetadata(path, to);
                             path = to;
                         } else displayError("Rename failed", true);
                     }
                 }                                          },
                {"Copy",
                 [&]() {
                     String to = newPath("");
                     String data;
                     if (to.length() && !fs.exists(to) && readText(fs, path, data)) safeSave(fs, to, data);
                 }                                          },
                {"Favorite ON/OFF",
                 [&]() {
                     String old = index["scripts"][path]["favorite"] | "0";
                     metadata(path, "favorite", old == "1" ? "0" : "1");
                 }                                          },
                {"Category",
                 [&]() {
                     String cat = textKeyboard(index["scripts"][path]["category"] | "", 32, "Category label");
                     metadata(path, "category", cat);
                 }                                          },
                {"Delete",
                 [&]() {
                     if (confirmation("Delete script permanently?")) {
                         if (!fs.remove(path)) displayError("Delete failed", true);
                         else {
                             index["scripts"].remove(path);
                             saveIndex();
                         }
                     }
                 }                                          },
                {"Back",            []() {}                 }
            };
            int n = loopOptions(menu, MENU_TYPE_SUBMENU, path.c_str());
            if (n < 0 || n == int(menu.size()) - 1 || !fs.exists(path)) return;
        }
    }
    void filtered(bool favorites) {
        std::vector<Option> list;
        std::vector<String> paths;
        JsonObject records = index["scripts"].as<JsonObject>();
        for (JsonPair pair : records) {
            String path = pair.key().c_str();
            String value = pair.value()[favorites ? "favorite" : "recent_uptime"] | "0";
            if ((favorites ? value == "1" : value != "0") && fs.exists(path)) paths.push_back(path);
        }
        if (!favorites)
            std::sort(paths.begin(), paths.end(), [&](const String &a, const String &b) {
                return (records[a]["order"] | 0u) > (records[b]["order"] | 0u);
            });
        for (const String &path : paths)
            list.push_back({path + " [" + String(records[path]["category"] | "") + "]", [&, path]() {
                                actions(path);
                            }});
        if (list.empty()) {
            displayInfo("No matching scripts", true);
            return;
        }
        loopOptions(list, MENU_TYPE_SUBMENU, favorites ? "Favorites" : "Recent (saved entries)");
    }
    void installTests() {
        const char *names[] = {
            "01-notepad.txt", "02-turkish-q.txt", "03-url.txt", "04-chords.txt", "05-delay-stop.txt"
        };
        const char *contents[] = {
            "REM Open visible Notepad and type TEST\nGUI r\nDELAY 1000\nSTRING notepad\nENTER\nDELAY "
            "1500\nSTRINGLN TEST\n",
            "REM Focus an empty Notepad first; Windows Turkish Q\nDELAY 2000\nSTRINGLN :/?=&-_ "
            "@#$[]{}()\nSTRINGLN çğıöşüÇĞİÖŞÜ\nSTRINGLN 0123456789 aAzZ\n",
            "REM Open example.com visibly; approve only on your computer\nGUI r\nDELAY 1200\nSTRING "
            "https://example.com/a-b?q=1&x=two\nENTER\n",
            "REM Visible shortcut test: open then cancel Run dialog\nGUI r\nDELAY 1000\nESCAPE\nDELAY "
            "500\nSTRINGLN TEST\n",
            "REM Test STOP during 30-second delay; no text should follow a stop\nDELAY 30000\nSTRINGLN Delay "
            "finished\n"
        };
        const String dir = "/badusb-pro-tests";
        if (!fs.exists(dir) && !fs.mkdir(dir)) {
            displayError("Cannot create test directory", true);
            return;
        }
        for (unsigned i = 0; i < 5; ++i) {
            String path = dir + "/" + names[i];
            if (!fs.exists(path) && !safeSave(fs, path, String(contents[i]))) return;
        }
        displayInfo("Tests installed; existing files preserved", true);
    }
    void run() {
        for (;;) {
            std::vector<Option> menu = {
                {"Browse scripts",
                 [&]() {
                     String path = loopSD(fs, true, "txt");
                     if (path.length()) actions(path);
                 }                                                 },
                {"New script",
                 [&]() {
                     String path = newPath("");
                     if (path.length() && !fs.exists(path)) editor(path, true);
                 }                                                 },
                {"Favorites",            [&]() { filtered(true); } },
                {"Recent scripts",       [&]() { filtered(false); }},
                {"Install benign tests",
                 [&]() {
                     if (confirmation("Write five visible test scripts?")) installTests();
                 }                                                 },
                {"Browse categories",
                 [&]() {
                     std::vector<Option> m;
                     JsonObject records = index["scripts"].as<JsonObject>();
                     for (JsonPair pair : records) {
                         String path = pair.key().c_str(), cat = pair.value()["category"] | "Uncategorized";
                         if (fs.exists(path))
                             m.push_back({cat + ": " + path, [&, path]() { actions(path); }});
                     }
                     if (m.size()) loopOptions(m, MENU_TYPE_SUBMENU, "Categories");
                 }                                                 },
                {"Back",                 []() {}                   }
            };
            int n = loopOptions(menu, MENU_TYPE_SUBMENU, "BadUSB Pro");
            if (n < 0 || n == int(menu.size()) - 1) return;
        }
    }
};
} // namespace
bool waveshareProRunning() { return running; }
bool waveshareProLineValid(const String &line, size_t at, size_t next) {
    return !running ||
           (approvedDocument &&
            wsScriptLineMatches(
                approvedDocument->c_str(), approvedDocument->length(), at, next, line.c_str(), line.length()
            ));
}
bool waveshareProStop(HIDInterface *hid) {
    if (!running) return false;
    if (aborted || check(EscPress) || !hid || !hid->isConnected()) {
        aborted = true;
        if (hid) hid->releaseAll();
        return true;
    }
    return false;
}
bool waveshareProWait(HIDInterface *hid, uint32_t ms) {
    if (!running) {
        delay(ms);
        return true;
    }
    uint32_t start = millis();
    do {
        if (waveshareProStop(hid)) return false;
        delay(10);
    } while (uint32_t(millis() - start) < ms);
    return true;
}
size_t waveshareProText(HIDInterface *hid, const String &s) {
    if (!running) return hid->print(s);
    size_t pos = 0;
    while (pos < s.length()) {
        if (waveshareProStop(hid)) break;
        size_t start = pos;
        uint32_t cp;
        if (!hidTextDecode((const uint8_t *)s.c_str(), s.length(), pos, cp)) return start;
        const size_t n = hid->write((const uint8_t *)s.c_str() + start, pos - start);
        if (n != pos - start) return start;
    }
    return pos;
}
void waveshareProProgress(HIDInterface *hid, size_t line, size_t at, size_t total) {
    if (!running || uint32_t(millis() - lastProgress) < 250) return;
    lastProgress = millis();
    const int oldX = tft.getCursorX(), oldY = tft.getCursorY();
    const uint8_t oldSize = tft.getTextSize();
    const uint16_t oldColor = tft.getTextColor(), oldBg = tft.getTextBgColor();
    tft.setTextSize(1);
    tft.fillRect(10, tftHeight - 68, tftWidth - 20, 28, bruceConfig.bgColor);
    tft.drawString(
        "Line " + String(line) + " / " + String(total ? at * 100 / total : 0) + "% USB " +
            String(hid && hid->isConnected() ? "OK" : "lost") + " | Back = STOP",
        12,
        tftHeight - 64
    );
    tft.setTextSize(oldSize);
    tft.setTextColor(oldColor, oldBg);
    tft.setCursor(oldX, oldY);
}
void waveshareBadUSBPro() {
    FS *fs = nullptr;
    if (!getFsStorage(fs) || !fs) return;
    Manager m(*fs);
    m.run();
}
#endif
