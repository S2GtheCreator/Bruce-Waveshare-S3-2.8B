#include "waveshare_support.h"
#include "core/scrollableTextArea.h"
#include "core/settings.h"
#include "core/settings.h"
#include "battery_model.h"
#include <Arduino.h>
#include <Preferences.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <globals.h>
#include "core/display.h"
#include "core/mykeyboard.h"
#include "freertos/semphr.h"
namespace {
SemaphoreHandle_t batteryMutex=nullptr;
WaveshareBatteryState state;
BatteryLowDebounce lowState;
float gain=1.0f;
uint32_t lastSample=0;
bool sampled=false;
void logStatus(const WaveshareBatteryState &s) {
    char line[256];
    snprintf(line, sizeof(line), "[BAT] GPIO4 ADC1, calibrated 12bit/11dB, divider=3; valid=%d voltage=%umV estimate=~%u%% low=%d; source=unknown charge=unknown",
        int(s.valid), unsigned(s.mv), unsigned(s.percent), int(s.low));
    // Do not forward va_list through the inherited USBSerial variadic printf adapter.
    serialDevice->println(String(line));
}
}
void waveshareBatteryInit() {
    batteryMutex=xSemaphoreCreateMutex();
    if (!batteryMutex) { Serial.println("[BAT] mutex allocation failed; battery UI disabled"); return; }
    pinMode(4,INPUT);
    analogReadResolution(12);
    // Keep Arduino's default attenuation and per-unit calibration in agreement.
    analogSetPinAttenuation(4,ADC_11db);
    Preferences prefs;
    if (prefs.begin("ws28b-bat",true)) { gain=prefs.getFloat("gain",1.0f); prefs.end(); }
    if (!std::isfinite(gain) || gain<0.9f || gain>1.1f) gain=1.0f;
    delay(25); // R2/R18 divider + C10 settles; setup only, before input task.
    waveshareBatteryService();
    logStatus(waveshareBatterySnapshot());
}
void waveshareBatteryService() {
    if (!batteryMutex || xSemaphoreTake(batteryMutex,0)!=pdTRUE) return;
    uint32_t now=millis();
    if (sampled && (uint32_t)(now-lastSample)<5000) { xSemaphoreGive(batteryMutex); return; }
    lastSample=now; sampled=true;
    uint32_t readings[16];
    bool valid=true;
    for (auto &v : readings) {
        // Arduino 3.3.9 uses IDF curve-fitting calibration; returns zero on error.
        v=analogReadMilliVolts(4);
        if (!v || v>3100) valid=false;
    }
    std::sort(readings,readings+16);
    uint32_t sum=0;
    for (int i=2;i<14;++i) sum+=readings[i]; // trimmed mean, reject transient extremes
    uint32_t mv=(uint32_t)std::lround((sum/12.0f)*3.0f*gain);
    valid=valid && mv>=2500 && mv<=4500;
    const bool oldLow=state.low;
    state.mv=valid ? mv : 0; state.valid=valid; state.sampledAt=now;
    state.percent=valid ? batteryEstimatePercent(mv) : 0;
    state.low=lowState.update(mv,valid);
    if (!valid) state.low=false; // do not turn a failed ADC into an empty-battery warning
    const bool report=state.low!=oldLow;
    WaveshareBatteryState copy=state;
    xSemaphoreGive(batteryMutex);
    if (report) logStatus(copy);
}
WaveshareBatteryState waveshareBatterySnapshot() {
    WaveshareBatteryState copy;
    if (!batteryMutex) return copy;
    if (xSemaphoreTake(batteryMutex,pdMS_TO_TICKS(25))==pdTRUE) {
        copy=state; xSemaphoreGive(batteryMutex);
        // Failed or stale readings must not masquerade as a calibrated battery value.
        if ((uint32_t)(millis()-copy.sampledAt)>15000) copy.valid=false;
    }
    return copy;
}
void waveshareBatteryMenu() {
    for (;;) {
        std::vector<Option> items={
            #if WS28B_BATTERY_PRO
            {"Display brightness", setBrightnessMenu},
            {"Screen timeout", setDimmerTimeMenu},
            {"Display brightness", setBrightnessMenu},
            {"Screen timeout", setDimmerTimeMenu},
            {"Battery dashboard", []() {
                const auto s=waveshareBatterySnapshot();
                String text="ADC measurement: "+String(s.valid?"valid":"unavailable/stale")+"\n";
                if(s.valid) text+="BAT terminal: "+String(s.mv/1000.0f,3)+" V\n";
                if(s.valid && waveshareBatteryConfirmed()) text+="Approximate: ~"+String(s.percent)+"% (user-confirmed battery, voltage estimate)\n";
                text+="Battery present: unknown\nPower source: unknown\nCharging: unknown\n";
                text+="Low voltage: "+String(s.valid?(s.low?"YES":"no"):"unknown")+"\n";
                text+="A valid ADC voltage does not prove a battery is connected: the charger may bias an empty BAT terminal. Confirm battery presence below before showing a percentage.\n";
                text+="Accuracy: divider 1% resistors + ADC + battery load/temperature; percentage is not a fuel gauge.\nSW1/Q2 controls battery power physically; there is no MCU latch or charger status pin. USB HID connected is not proof of USB power source.\n";
                text+="Unplug USB with SW1 ON. If it shuts down, measure BAT, BAT_OUT, VCC and 3V3. Do not bridge protections.\n";
                ScrollableTextArea area("Battery Manager");area.fromString(text);area.show();
            }},
            {"Battery presence", []() {
                std::vector<Option> v={{"Unconfirmed / no battery", [](){Preferences n;bool ok=n.begin("ws28b-bat",false);if(ok){ok=n.putBool("confirmed",false)==1;n.end();}if(!ok)displayError("Battery presence save failed",true);}},
                    {"I verified a battery is fitted", [](){Preferences n;bool ok=n.begin("ws28b-bat",false);if(ok){ok=n.putBool("confirmed",true)==1;n.end();}if(!ok)displayError("Battery presence save failed",true);}}};
                loopOptions(v,MENU_TYPE_SUBMENU,"Physical confirmation");
            }},
#endif
            {"Voltage / estimate", []() {
                const auto s=waveshareBatterySnapshot();
                String text=s.valid ? String(s.mv/1000.0f,3)+"V BAT terminal. "+(waveshareBatteryConfirmed()?String("~")+String(s.percent)+"%. ":String("Battery not confirmed. "))+(s.low?"LOW. ":"") : String("ADC unavailable. ");
                text+="USB/battery source and charging cannot be auto-detected.";
                displayTextLine(text,true);
            }},
            {"Calibrate with meter", []() {
                const auto before=waveshareBatterySnapshot();
                if (!before.valid) { displayTextLine("ADC unavailable; cannot calibrate.",true); return; }
                String input=keyboard(String(before.mv/1000.0f,3),8,"Measured BAT voltage (V)");
                char *end=nullptr;
                float reference=strtof(input.c_str(),&end);
                if (end==input.c_str() || *end || !std::isfinite(reference) || reference<2.5f || reference>4.5f) {
                    displayTextLine("Invalid measured voltage.",true); return;
                }
                if (!batteryMutex || xSemaphoreTake(batteryMutex,pdMS_TO_TICKS(100))!=pdTRUE) return;
                const uint32_t currentMv=state.mv;
                float next=currentMv ? gain*reference*1000.0f/currentMv : 0;
                if (!state.valid || (uint32_t)(millis()-state.sampledAt)>15000) next=0;
                bool ok=std::isfinite(next) && next>=0.9f && next<=1.1f;
                Preferences prefs;
                if (ok) {
                    ok=prefs.begin("ws28b-bat",false);
                    if (ok) { ok=prefs.putFloat("gain",next)==sizeof(float); prefs.end(); }
                }
                if (ok) { gain=next; sampled=false; }
                xSemaphoreGive(batteryMutex);
                if(ok) waveshareBatteryService();
                displayTextLine(ok?"Meter gain saved. Estimate is not a fuel gauge.":"Calibration rejected or NVS write failed.",true);
            }},
            {"Power switch info", []() {
                displayTextLine("SW1 switches BAT to BAT_OUT. No MCU power-cut pin. USB can keep VCC powered. Charging LED is hardware-only.",true);
            }},
            {"Back", [](){}}
        };
        int selection=loopOptions(items,MENU_TYPE_SUBMENU,"Battery Manager");
        if(selection<0 || selection==int(items.size())-1) return;
    }
}
uint32_t wsBatteryCallback(cmd *) {
    logStatus(waveshareBatterySnapshot()); return true;
}


bool waveshareBatteryConfirmed() {
#if WS28B_BATTERY_PRO
    Preferences n; bool fitted=false;
    if(n.begin("ws28b-bat",true)){fitted=n.getBool("confirmed",false);n.end();}
    return fitted;
#else
    return true;
#endif
}
