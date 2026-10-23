"""Compile actual USB keyboard implementation against a recording transport.
No USB packets are sent. Compare reports with frozen Windows layout API data.
HOST_CXX may point to another C++11 host compiler.
"""
from pathlib import Path
import json,os,subprocess,tempfile,shutil
root=Path(__file__).resolve().parents[2];lib=root/'lib/Bad_Usb_Lib'
reference=json.loads((root/'boards/waveshare_28b/turkish-q-reference.json').read_text(encoding='utf-8'))
compiler=os.environ.get('HOST_CXX','C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe')
cases=[]
for layout in ['tr','us']:
 for caps in [False,True]:
  for cp in range(32,127):cases.append((layout,caps,chr(cp).encode(),0,cp))
  if layout=='tr':
   for key in reference['tr']:
    if int(key)>=128:cases.append((layout,caps,chr(int(key)).encode(),0,int(key)))
for data,failure in [(b'prefix\xf0\x9f\x99\x82',2),(b'\xc0\xaf',1),(b'\xed\xa0\x80',1),
 (b'\xf4\x90\x80\x80',1),(b'\xe2\x82',1),(b'\x80',1),(b'\x00',2)]:
 cases.append(('tr',False,data,failure,None))
cases.append(('us',False,'ç'.encode(),2,None))
for layout in ['tr','us']:
 for text in [':/?=&-_','@#$[]{}()','0123456789 AaZz Ii Qq Yy','https://example.com/a-b?q=1&x=two',
              "Write-Output 'A:/?=&-_@#$[]{}()'",'^`~abcdef']:
  cases.append((layout,False,text.encode(),0,None))
cases.append(('tr',False,'çğıöşüÇĞİÖŞÜ'.encode(),0,None))
cases.extend((layout,False,b'abc\r\n\t\b',0,None) for layout in ['tr','us'])
with tempfile.TemporaryDirectory(prefix='bruce-trq-') as tmp:
 t=Path(tmp)
 for name in ['USBHIDKeyboard.cpp','USBHIDKeyboard.h','Bad_Usb_Lib.h','keys.h','KeyboardLayout.h',
              'KeyboardLayout_en_US.cpp','KeyboardLayout_tr_TR.cpp','HidText.h','HidTextTrMap.h']:
  shutil.copy2(lib/name,t/name)
 (t/'Arduino.h').write_text(r'''
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <cstring>
#define PROGMEM
inline void delay(unsigned){}
class Print {public:virtual ~Print(){};virtual size_t write(uint8_t){return 0;}
virtual size_t write(const uint8_t*,size_t){return 0;} void setWriteError(){};void clearWriteError(){};};
class Stream:public Print {};
struct SerialMock {template<class T>void print(T){}template<class T>void println(T){}};
extern SerialMock Serial;
''')
 (t/'Print.h').write_text('#pragma once\n#include "Arduino.h"\n')
 (t/'esp_event.h').write_text(r'''
#pragma once
#include <stdint.h>
typedef int esp_err_t;typedef unsigned TickType_t;typedef const char* esp_event_base_t;
typedef void(*esp_event_handler_t)(void*,esp_event_base_t,int32_t,void*);
#define ESP_EVENT_ANY_ID -1
#define ESP_EVENT_DECLARE_BASE(x) extern const char *x
#define ESP_EVENT_DEFINE_BASE(x) const char *x=#x
#define portMAX_DELAY 0
''')
 (t/'USBHID.h').write_text(r'''
#pragma once
#include "Arduino.h"
#include "esp_event.h"
#include <vector>
#define CONFIG_TINYUSB_HID_ENABLED 1
#define HID_ITF_PROTOCOL_KEYBOARD 1
#define HID_REPORT_ID_KEYBOARD 1
#define HID_KEY_SHIFT_LEFT 0xE1
#define HID_REPORT_ID(x) x
#define TUD_HID_REPORT_DESC_KEYBOARD(x) 1
struct hid_keyboard_report_t {uint8_t modifier,reserved,keycode[6];};
extern std::vector<hid_keyboard_report_t> reports;
class USBHIDDevice {public: virtual uint16_t _onGetDescriptor(uint8_t*){return 0;}};
class USBHID {public:USBHID(int=0){}void begin(){};void addDevice(USBHIDDevice*,unsigned){}
void SendReport(unsigned,const void*data,size_t){hid_keyboard_report_t r;memcpy(&r,data,sizeof(r));reports.push_back(r);}};
''')
 (t/'tusb.h').write_text('#pragma once\ninline bool tud_mounted(){return true;}\ninline bool tud_suspended(){return false;}\n')
 (t/'harness.cpp').write_text(r'''
#include "USBHIDKeyboard.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
SerialMock Serial;std::vector<hid_keyboard_report_t> reports;
esp_err_t arduino_usb_event_post(esp_event_base_t,int32_t,void*,size_t,TickType_t){return 0;}
esp_err_t arduino_usb_event_handler_register_with(esp_event_base_t,int32_t,esp_event_handler_t,void*){return 0;}
int main(int argc,char**argv){
 USBHIDKeyboard k;k.setDelay(0);std::ifstream input(argv[1]);std::string row;
 while(std::getline(input,row)){
  std::istringstream in(row);int tr,caps;std::string hex;in>>tr>>caps>>hex;
  std::vector<uint8_t> bytes;for(size_t i=0;i<hex.size();i+=2)bytes.push_back(std::strtoul(hex.substr(i,2).c_str(),nullptr,16));
  k.begin(tr?KeyboardLayout_tr_TR:KeyboardLayout_en_US);k.releaseAll();
  uint8_t led=caps?2:0;k._onOutput(1,&led,1);reports.clear();
  size_t n=k.write(bytes.data(),bytes.size());
  const auto&e=k.textError();std::cout<<n<<" "<<int(e.failure)<<" "<<e.codepoint<<" "<<e.offset;
  for(const auto&r:reports)std::cout<<" "<<int(r.modifier)<<":"<<int(r.keycode[0]);
  std::cout<<"\n";
 }
 k.begin(KeyboardLayout_tr_TR);uint8_t off=0;k._onOutput(1,&off,1);
 for(uint8_t mod:{uint8_t(KEY_LEFT_GUI),uint8_t(KEY_LEFT_CTRL),uint8_t(KEY_LEFT_ALT)}){
  k.releaseAll();reports.clear();k.press(mod);k.press('i');
  assert(reports.back().keycode[0]==0x0C);k.releaseAll();assert(reports.back().modifier==0);
 }
 k.releaseAll();reports.clear();k.press(KEY_LEFT_GUI);k.press('r');assert(reports.back().keycode[0]==0x15&&reports.back().modifier==8);
 k.releaseAll();k.press(KEY_LEFT_CTRL);k.press('i');k.release(KEY_LEFT_CTRL);k.release('i');
 assert(reports.back().modifier==0&&reports.back().keycode[0]==0); // Modifier may be released first.
 k.releaseAll();k.press(KEY_LEFT_ALT);k.press(KEY_F4);assert(reports.back().keycode[0]==0x3D&&reports.back().modifier==4);
 k.releaseAll();k.press(KEY_LEFT_SHIFT);k.press(KEY_RETURN);assert(reports.back().keycode[0]==0x28&&reports.back().modifier==2);
 k.releaseAll();reports.clear();assert(k.write(uint8_t(KEY_RETURN))==1&&reports.front().keycode[0]==0x28);
 k.releaseAll();k.press(KEY_LEFT_CTRL);reports.clear();const uint8_t x='x';k.write(&x,1);
 assert(reports.front().modifier==1&&reports.front().keycode[0]==0x1B&&reports.back().modifier==1);
 k.releaseAll();for(uint8_t raw=4;raw<10;++raw)k.pressRaw(raw);reports.clear();
 const uint8_t q='q';assert(k.write(&q,1)==0&&k.textError().failure==HidTextFailure::FullReport&&reports.empty());
 k.releaseAll();
 // Malformed host output reports must not dereference an empty buffer.
 k._onOutput(1,nullptr,0);
}
''')
 (t/'input.txt').write_text(''.join(f'{int(layout=="tr")} {int(caps)} {data.hex()}\n' for layout,caps,data,_,_ in cases))
 cmd=[compiler,'-std=c++11','-DWAVESHARE_28B=1','-I'+str(t),str(t/'USBHIDKeyboard.cpp'),
      str(t/'KeyboardLayout_en_US.cpp'),str(t/'KeyboardLayout_tr_TR.cpp'),str(t/'harness.cpp'),'-o',str(t/'test.exe')]
 p=subprocess.run(cmd,capture_output=True,text=True);assert p.returncode==0,p.stderr
 p=subprocess.run([str(t/'test.exe'),str(t/'input.txt')],capture_output=True,text=True);assert p.returncode==0,p.stderr
 rows=p.stdout.splitlines();assert len(rows)==len(cases),(len(rows),len(cases),p.stdout[-500:])
 for row,(layout,caps,data,failure,cp) in zip(rows,cases):
  values=row.split();written,kind,errcp,at=map(int,values[:4]);frames=[tuple(map(int,x.split(':'))) for x in values[4:]]
  assert kind==failure,(layout,caps,data,row)
  if failure:
   assert written==0 and not frames,(data,row) # Whole-buffer preflight: no prefix sent.
   continue
  assert written==len(data)
  expected=[]
  for ch in data.decode():
   if ch=='\r':continue
   if ch in '\n\t\b':
    expected.extend([(0,{'\n':0x28,'\t':0x2b,'\b':0x2a}[ch]),(0,0)]);continue
   e=reference[layout+('_caps' if caps else '')][str(ord(ch))]
   expected.extend([(e['modifiers'],e['usage']),(0,0)])
   if e['dead']:expected.extend([(0,0x2c),(0,0)])
  assert frames==expected,(layout,caps,data,frames,expected)
 print('PASS: actual USBHIDKeyboard.cpp + shared codec,',len(cases),'recorded report cases match frozen Windows layout API data')
 print('PASS: ASCII 95, Turkish 12 and 10 extra Unicode, Caps on/off, Shift/AltGr, dead-key completion and English US return')
 print('PASS: GUI/CTRL/ALT + i, GUI+r, ALT+F4, SHIFT+ENTER, raw ENTER and empty host output report')
 print('PASS: unsupported Unicode / US Turkish / malformed UTF-8 / NUL rejected before any prefix report')
