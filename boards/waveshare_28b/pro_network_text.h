#pragma once
#include <Arduino.h>
#include <HidText.h>

// Keep an AP name on one visual line and keep exports valid UTF-8.
// Literal backslashes are escaped too, so \xNN is an unambiguous byte escape.
inline String wsNetworkLabel(const String &raw) {
    String out;
    if (raw.isEmpty()) return out;
    if (!out.reserve(raw.length() * 4 + 1)) return "[SSID allocation failed]";
    for (size_t pos = 0; pos < raw.length();) {
        const size_t start = pos;
        uint32_t cp = 0;
        const bool valid = hidTextDecode((const uint8_t *)raw.c_str(), raw.length(), pos, cp);
        if (!valid) pos = start + 1;
        if (!valid || cp < 32 || cp == 127 || cp == '\\') {
            char escaped[5];
            snprintf(escaped, sizeof(escaped), "\\x%02X", uint8_t(raw[start]));
            out += escaped;
        } else out += raw.substring(start, pos);
    }
    return out;
}

inline String wsCsvField(const String &raw) {
    String value = wsNetworkLabel(raw);
    size_t first = 0;
    while (first < value.length() && uint8_t(value[first]) <= 32) ++first;
    if (first < value.length() && String("=+-@").indexOf(value[first]) >= 0) value = "'" + value;
    value.replace("\"", "\"\"");
    return "\"" + value + "\"";
}
