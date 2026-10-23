#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const size_t WS_SCRIPT_MAX_BYTES = 16384;
static const size_t WS_SCRIPT_MAX_LINES = 128;
static const size_t WS_SCRIPT_MAX_LINE_BYTES = 512;
enum class WsScriptLimit { None, Bytes, Lines, LineBytes };

// Check before splitting into heap-allocated String/vector entries.
inline WsScriptLimit wsScriptValidate(const char *data, size_t size, size_t &line) {
    line = size ? 1 : 0;
    if (size > WS_SCRIPT_MAX_BYTES || (size && !data)) return WsScriptLimit::Bytes;
    size_t bytes = 0;
    for (size_t pos = 0; pos < size; ++pos) {
        if (data[pos] == '\n') {
            bytes = 0;
            if (pos + 1 < size && ++line > WS_SCRIPT_MAX_LINES) return WsScriptLimit::Lines;
        } else if (data[pos] == '\r' && pos + 1 < size && data[pos + 1] == '\n') {
            // CRLF terminator is not part of the editable line.
        } else if (++bytes > WS_SCRIPT_MAX_LINE_BYTES) return WsScriptLimit::LineBytes;
    }
    return WsScriptLimit::None;
}

// A damaged/short filesystem read must never execute a partial approved line.
inline bool wsScriptLineMatches(const char *approved, size_t size, size_t at, size_t next,
                                const char *actual, size_t length) {
    if (!approved || at >= size || next > size || (!actual && length)) return false;
    size_t end = at;
    while (end < size && approved[end] != '\n') ++end;
    const size_t expectedNext = end < size ? end + 1 : end;
    return next == expectedNext && length == end - at &&
           (!length || memcmp(approved + at, actual, length) == 0);
}
