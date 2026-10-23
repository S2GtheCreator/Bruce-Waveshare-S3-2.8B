#pragma once
#include <Arduino.h>
#ifndef WS28B_WIFI_ANALYZER_PRO
#define WS28B_WIFI_ANALYZER_PRO 1
#endif
#ifndef WS28B_BADUSB_PRO
#define WS28B_BADUSB_PRO 1
#endif
#include <stddef.h>
#include <stdint.h>
class HIDInterface;
#if WS28B_WIFI_ANALYZER_PRO
void waveshareAnalyzerPro();
#endif
#if WS28B_BADUSB_PRO
void waveshareBadUSBPro();
bool waveshareProRunning();
bool waveshareProStop(HIDInterface *);
bool waveshareProLineValid(const String &, size_t, size_t);
bool waveshareProWait(HIDInterface *, uint32_t);
size_t waveshareProText(HIDInterface *, const String &);
void waveshareProProgress(HIDInterface *, size_t, size_t, size_t);
#endif

#if WS28B_TOUCH_PRO
String waveshareTextKeyboard(const String &, size_t, const String &, bool *cancelled = nullptr);
void waveshareResources();
#endif
