#pragma once
#include <stdint.h>
#include <SimpleCLI.h>
struct WaveshareBatteryState {
    uint32_t mv=0, sampledAt=0;
    uint8_t percent=0;
    bool valid=false, low=false;
};
void waveshareBatteryInit();
void waveshareBatteryService();
WaveshareBatteryState waveshareBatterySnapshot();
bool waveshareTouchSuppressed();
void waveshareDiagnosticsMenu();
void waveshareBatteryMenu();
void waveshareRunPendingDiagnostic();
void createWaveshareCommands(SimpleCLI *);

#ifndef WS28B_BATTERY_PRO
#define WS28B_BATTERY_PRO 1
#endif
bool waveshareBatteryConfirmed();
