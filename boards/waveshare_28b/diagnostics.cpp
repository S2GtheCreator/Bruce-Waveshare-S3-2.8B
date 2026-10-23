#include "waveshare_support.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#include <atomic>
#include <globals.h>
#include "core/display.h"
#include "core/utils.h"
#include "core/wifi/webInterface.h"
namespace {
std::atomic<bool> noTouch(false);
std::atomic<int> pending(-1);
const char *const modeNames[]={"Fixed image / current radio","Fixed image / touch polling OFF","Fixed image / WiFi OFF","Fixed image / WiFi scan ON"};
void runDiagnostic(int mode) {
    if (mode<0 || mode>3) return;
    wifi_mode_t savedMode=WiFi.getMode();
    if ((mode==2 || mode==3) && (isWebUIActive || savedMode==WIFI_AP || savedMode==WIFI_AP_STA)) {
        Serial.println("[LCD TEST] Stop AP/WebUI before radio isolation; current AP not changed.");
        displayTextLine("Stop AP/WebUI before radio isolation test.",false);
        return;
    }
    if (mode==3 && WiFi.scanComplete()==WIFI_SCAN_RUNNING) {
        Serial.println("[LCD TEST] A WiFi scan is already active; test not started."); return;
    }
    const bool connected=WiFi.status()==WL_CONNECTED;
    const String ssid=connected ? WiFi.SSID() : String("");
    const String password=connected ? WiFi.psk() : String("");
    bool radioChanged=(mode==2 || mode==3);
    bool radioOK=true;
    if(mode==2) radioOK=WiFi.mode(WIFI_OFF);
    if(mode==3) radioOK=WiFi.mode(WIFI_STA);
    if(!radioOK) {
        Serial.println("[LCD TEST] Radio mode change failed");
        WiFi.mode(savedMode); return;
    }
    wakeUpScreen();
    previousMillis=millis();
    noTouch.store(mode==1,std::memory_order_relaxed);
    // One draw only. No UI refresh, buffer swap or periodic LCD restart for 60 seconds.
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(FM); tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.drawCentreString("Waveshare RGB baseline",tftWidth/2,35);
    tft.drawCentreString(modeNames[mode],tftWidth/2,70);
    uint16_t colors[]={TFT_WHITE,TFT_RED,TFT_GREEN,TFT_BLUE,TFT_YELLOW,TFT_CYAN};
    int top=120, barHeight=std::max(1,(tftHeight-top-70)/6);
    for(int i=0;i<6;++i) tft.fillRect(20,top+i*barHeight,tftWidth-40,barHeight,colors[i]);
    tft.drawCentreString("60 sec; BOOT exits",tftWidth/2,tftHeight-40);
    Serial.printf("[LCD TEST] %s; framebuffer=1 bounce=4800px PCLK=30MHz; BLE unchanged\n",modeNames[mode]);
    uint32_t start=millis(),lastScan=0,lastLog=0;
    // Enter via touch/CLI after boot. Holding BOOT during reset selects ROM downloader.
    while(digitalRead(0)==LOW && (uint32_t)(millis()-start)<2000) delay(10);
    while((uint32_t)(millis()-start)<60000 && digitalRead(0)!=LOW) {
        previousMillis=millis(); // prevent display dimming during controlled test
        if(mode==3 && (lastScan==0 || (uint32_t)(millis()-lastScan)>=10000)) {
            int n=WiFi.scanComplete();
            if(n!=WIFI_SCAN_RUNNING) { WiFi.scanDelete(); WiFi.scanNetworks(true); lastScan=millis(); }
        }
        if ((uint32_t)(millis()-lastLog)>=5000) {
            const auto bat=waveshareBatterySnapshot();
            Serial.printf("[LCD TEST] elapsed=%us PSRAMfree=%u largest=%u BAT=%umV valid=%d WiFiMode=%d\n",
                unsigned((millis()-start)/1000),unsigned(ESP.getFreePsram()),unsigned(ESP.getMaxAllocPsram()),unsigned(bat.mv),bat.valid,int(WiFi.getMode()));
            lastLog=millis();
        }
        // Sleep/wake can have registered the main UI task with TWDT.
        // Keep that watchdog alive during a bounded diagnostic, without changing its policy.
        if (esp_task_wdt_status(nullptr)==ESP_OK) esp_task_wdt_reset();
        delay(20);
    }
    noTouch.store(false,std::memory_order_relaxed);
    if(mode==3) { esp_wifi_scan_stop(); WiFi.scanDelete(); }
    if(radioChanged) {
        WiFi.mode(savedMode);
        if(connected) WiFi.begin(ssid.c_str(),password.c_str()); // reconnect asynchronously; credentials not rewritten
    }
    previousMillis=millis(); EscPress=false; AnyKeyPress=false;
    Serial.println("[LCD TEST] End; touch restored, radio mode restored (STA reconnection may still be pending).");
}
uint32_t queueMode(int mode) { pending.store(mode); serialDevice->println("[LCD TEST] Queued for main UI task; BOOT exits or 60s timeout."); return true; }
uint32_t fixedCallback(cmd *){return queueMode(0);}
uint32_t touchCallback(cmd *){return queueMode(1);}
uint32_t offCallback(cmd *){return queueMode(2);}
uint32_t onCallback(cmd *){return queueMode(3);}
}
bool waveshareTouchSuppressed(){return noTouch.load(std::memory_order_relaxed);}
void waveshareRunPendingDiagnostic(){
    int mode=pending.exchange(-1);if(mode>=0)runDiagnostic(mode);
}
void waveshareDiagnosticsMenu(){
    std::vector<Option> items={
        {"Fixed image", [](){runDiagnostic(0);}},
        {"Touch polling OFF", [](){runDiagnostic(1);}},
        {"WiFi OFF", [](){runDiagnostic(2);}},
        {"WiFi scan ON", [](){runDiagnostic(3);}},
        {"Back", [](){}}
    };
    loopOptions(items,MENU_TYPE_SUBMENU,"LCD Diagnostics");
}
extern uint32_t wsBatteryCallback(cmd *);
void createWaveshareCommands(SimpleCLI *cli) {
    cli->addCommand("ws28b-fixed",fixedCallback);
    cli->addCommand("ws28b-touch-off",touchCallback);
    cli->addCommand("ws28b-wifi-off",offCallback);
    cli->addCommand("ws28b-wifi-on",onCallback);
    cli->addCommand("ws28b-battery",wsBatteryCallback);
}

