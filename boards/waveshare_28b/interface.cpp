#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <esp_heap_caps.h>
#include <globals.h>
#include "waveshare_support.h"
#include "core/bus_HAL.h"
#include "core/powerSave.h"
#include "core/utils.h"
#include "core/display.h"
namespace {
bool busReady=false, touchReady=false, panelReady=false;
uint8_t exOutput=0;
uint16_t touchMaxX=480, touchMaxY=640;
[[noreturn]] void fatal(const char *reason) {
    digitalWrite(6,LOW);
    Serial.printf("[waveshare_28b] FATAL: %s. Reset after correcting hardware/configuration.\n",reason);
    Serial.flush();
    while(true) delay(1000);
}
bool exWrite(uint8_t reg,uint8_t val) {
    if(!busReady) return false;
    Wire.beginTransmission(0x20); Wire.write(reg); Wire.write(val);
    return Wire.endTransmission()==0;
}
bool exPin(uint8_t bit,bool level) {
    uint8_t next=level ? exOutput | (1U<<bit) : exOutput & ~(1U<<bit);
    if(!exWrite(1,next)) return false;
    exOutput=next; return true;
}
bool readTouch(uint16_t reg,uint8_t *data,size_t n) {
    if(!busReady) return false;
    Wire.beginTransmission(0x5D); Wire.write(uint8_t(reg>>8)); Wire.write(uint8_t(reg));
    if(Wire.endTransmission(false)!=0) return false;
    if(Wire.requestFrom(uint8_t(0x5D),n)!=n) return false;
    for(size_t i=0;i<n;++i) data[i]=Wire.read();
    return true;
}
bool ackTouch() {
    Wire.beginTransmission(0x5D); Wire.write(uint8_t(0x81)); Wire.write(uint8_t(0x4E));
    Wire.write(uint8_t(0)); return Wire.endTransmission()==0;
}
// 9-bit mode-0 SPI. Release GPIO1/2 before SDMMC takes ownership.
void send9(bool data,uint8_t val) {
    uint16_t word=(uint16_t(data)<<8)|val;
    for(int bit=8;bit>=0;--bit) {
        digitalWrite(2,LOW); digitalWrite(1,(word>>bit)&1);
        delayMicroseconds(1); digitalWrite(2,HIGH); delayMicroseconds(1);
    }
    digitalWrite(2,LOW);
}
void ST7701_WriteCommand(uint8_t c) { send9(false,c); }
void ST7701_WriteData(uint8_t d) { send9(true,d); }
}
void _setup_gpio() {
    pinMode(6,OUTPUT); digitalWrite(6,LOW); pinMode(0,INPUT_PULLUP);
    if(!psramFound() || ESP.getPsramSize()<8*1024*1024) fatal("8MB OPI PSRAM not available");
    Serial.printf("[waveshare_28b] Flash=%u PSRAM=%u largest PSRAM=%u\n",
        unsigned(ESP.getFlashChipSize()),unsigned(ESP.getPsramSize()),
        unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM)));
    if(ESP.getFlashChipSize()!=16*1024*1024) fatal("Expected 16MB flash");
    setSysI2CBus(&Wire);
    busReady=Wire.begin(15,7,400000); Wire.setTimeOut(50);
    if(!busReady) fatal("I2C begin failed");
    // Official demo: all outputs, EXIO8 low. Set latch before direction.
    if(!exWrite(1,0) || !exWrite(3,0)) fatal("TCA9554 absent or write failed");
    // EXIO1 LCD reset bit0; EXIO2 touch reset bit1; EXIO3 LCD CS bit2;
    // EXIO4 SD D3 bit3. Official EXIO indices are 1-based.
    if(!exPin(2,true) || !exPin(0,false)) fatal("LCD reset failed");
    delay(10); if(!exPin(0,true)) fatal("LCD reset release failed"); delay(50);
    pinMode(1,OUTPUT); pinMode(2,OUTPUT); digitalWrite(2,LOW);
    if(!exPin(2,false)) fatal("LCD CS failed");
    delay(10);
#include "st7701_init.inc"
    if(!exPin(2,true)) fatal("LCD CS release failed");
    delay(10);
    pinMode(1,INPUT); pinMode(2,INPUT);
    // Official GT911 sequence selects 0x5D.
    pinMode(16,OUTPUT); digitalWrite(16,LOW);
    if(!exPin(1,false)) fatal("GT911 reset failed");
    delay(10);
    if(!exPin(1,true)) fatal("GT911 reset release failed");
    delay(200);
    digitalWrite(16,HIGH); pinMode(16,INPUT);
    uint8_t id[4]={}, resolution[4]={};
    touchReady=readTouch(0x8140,id,4) && id[0]=='9' && id[1]=='1' && id[2]=='1'
        && readTouch(0x8146,resolution,4);
    if(touchReady) {
        touchMaxX=resolution[0]|(resolution[1]<<8); touchMaxY=resolution[2]|(resolution[3]<<8);
        touchReady=touchMaxX>1 && touchMaxY>1;
    }
    Serial.printf("[waveshare_28b] GT911 ready=%d native=%ux%u\n",touchReady,touchMaxX,touchMaxY);
    if(!touchReady) Serial.println("[waveshare_28b] Touch unavailable; BOOT returns to menu.");
    waveshareBatteryInit();
    bruceConfig.colorInverted=0;
}
void _pre_storage_gpio() {
    panelReady=true;
    if(!exPin(3,true)) fatal("SD D3 pull-up failed");
    delay(10);
    if(!SD.setPins(2,1,42)) fatal("SDMMC pin configuration failed");
}
void _post_setup_gpio() {
    if(!ledcAttach(6,20000,10)) fatal("Backlight PWM attach failed");
}
void _setBrightness(uint8_t percent) {
    if(panelReady) ledcWrite(6,uint32_t(min(uint8_t(100),percent))*1023/100);
}
int getBattery() { return waveshareBatterySnapshot().percent; }
void InputHandler() {
    waveshareBatteryService();
    static bool wasPressed=false, bootWasPressed=false;
    bool bootPressed=digitalRead(0)==LOW;
    if(bootPressed && !bootWasPressed) { EscPress=true; AnyKeyPress=true; }
    bootWasPressed=bootPressed;
    if(!touchReady || waveshareTouchSuppressed()) return;
    uint8_t status=0, point[8]={};
    if(!readTouch(0x814E,&status,1) || !(status&0x80)) return;
    uint8_t count=status&0x0F;
    bool pressed=count>0 && count<=5;
    bool readOk=!pressed || readTouch(0x814F,point,8);
    if(!ackTouch() || !readOk) return;
    if(!pressed) { wasPressed=false; touchPoint.pressed=false; return; }
    if(wasPressed) return;
    wasPressed=true;
    uint16_t rawX=point[1]|(point[2]<<8),rawY=point[3]|(point[4]<<8);
    if(rawX>=touchMaxX || rawY>=touchMaxY) return;
    int x=uint32_t(rawX)*480/touchMaxX, y=uint32_t(rawY)*640/touchMaxY;
    switch(bruceConfigPins.rotation&3) {
        case 1: { int old=x; x=y; y=479-old; break; }
        case 2: x=479-x; y=639-y; break;
        case 3: { int old=x; x=639-y; y=old; break; }
    }
    if(wakeUpScreen()) return;
    AnyKeyPress=true;
    touchPoint.x=x; touchPoint.y=y; touchPoint.pressed=true;
    touchHeatMap(touchPoint); // Original Bruce navigation and bottom controls.
}
void powerOff() {
    Serial.println("[waveshare_28b] No software power-cut GPIO. SW1 controls battery path; USB can still supply VCC.");
    displayTextLine("Use physical battery switch. Unplug USB to remove USB power.", false);
}
void checkReboot() {}
