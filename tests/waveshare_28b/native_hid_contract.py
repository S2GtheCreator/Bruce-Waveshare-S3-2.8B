"""Host checks for real USB startup/cleanup branches; no hardware claim.
Usage: python tests/waveshare_28b/native_hid_contract.py [baseline-project]
"""
from pathlib import Path
import hashlib, json, os, subprocess, sys, tempfile

root=Path(__file__).resolve().parents[2]
baseline=Path(sys.argv[1]) if len(sys.argv)>1 else root.parent/'bruce-1.16.1-waveshare-recovery-battery'
allowed={
 'boards/_boards_json/waveshare_28b.json','boards/waveshare_28b/waveshare_28b.ini',
 'boards/waveshare_28b/pins_arduino.h','src/modules/badusb_ble/ducky_typer.cpp',
 'src/modules/badusb_ble/ducky_typer.h','lib/Bad_Usb_Lib/USBHIDKeyboard.cpp',
 'lib/Bad_Usb_Lib/USBHIDKeyboard.h','src/core/sd_functions.cpp',
 'src/core/serial_commands/badusb_commands.cpp','src/core/menu_items/OthersMenu.cpp',
 'src/core/menu_items/FileMenu.cpp','src/core/startup_app.cpp',
 'src/core/USBSerial/USBSerial.h','src/main.cpp','include/globals.h',
}
if baseline.is_dir():
 for folder in ['src','lib','boards','include']:
  for p in (baseline/folder).rglob('*'):
   if not p.is_file() or p.suffix not in {'.cpp','.c','.h','.hpp','.inc','.ini','.csv','.S'}: continue
   rel=p.relative_to(baseline).as_posix()
   if rel in allowed: continue
   assert (root/rel).is_file() and (root/rel).read_bytes()==p.read_bytes(),rel
 assert (root/'custom_16Mb.csv').read_bytes()==(baseline/'custom_16Mb.csv').read_bytes()
else:
 manifest=json.loads((root/'boards/waveshare_28b/native-hid-baseline-sha256.json').read_text())
 for rel,digest in manifest.items():
  if rel not in allowed:
   assert hashlib.sha256((root/rel).read_bytes()).hexdigest()==digest,rel
print('PASS: baseline display/touch/battery/WiFi/BLE and all runtime source outside declared USB changes preserved')

board=json.loads((root/'boards/_boards_json/waveshare_28b.json').read_text())
flags=board['build']['extra_flags']
assert '-DARDUINO_USB_MODE=0' in flags and '-DARDUINO_USB_MODE=1' not in flags
ini=(root/'boards/waveshare_28b/waveshare_28b.ini').read_text()
assert '-DUSB_as_HID=1' in ini and '-DARDUINO_USB_CDC_ON_BOOT=1' in ini
source=(root/'src/modules/badusb_ble/ducky_typer.cpp').read_text()
assert 'static USBHIDKeyboard waveshareUsbKeyboard;' in source
for p in [root/'src/core/sd_functions.cpp',root/'src/core/serial_commands/badusb_commands.cpp',root/'src/modules/badusb_ble/ducky_typer.cpp']:
 assert 'delete hid_usb;' not in p.read_text(),p
print('PASS: native OTG+CDC flags, early persistent keyboard and USB cleanup call sites')

compiler=os.environ.get('HOST_CXX','C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe')
def function(text, signature):
 start=text.index(signature); brace=text.index('{',start); depth=1; end=brace+1
 while depth:
  if text[end]=='{': depth+=1
  elif text[end]=='}': depth-=1
  end+=1
 return text[start:end]

with tempfile.TemporaryDirectory(prefix='bruce-hid-') as tmp:
 t=Path(tmp); (t/'soc').mkdir()
 (t/'soc/soc_caps.h').write_text('#define SOC_USB_OTG_SUPPORTED 1\n')
 (t/'sdkconfig.h').write_text('#define CONFIG_TINYUSB_ENABLED 1\n#define CONFIG_TINYUSB_HID_ENABLED 1\n#define CONFIG_TINYUSB_CDC_ENABLED 1\n')
 test=t/'guard.cpp'
 test.write_text('#include "'+(root/'boards/waveshare_28b/pins_arduino.h').as_posix()+'"\n')
 flags=['-DUSB_as_HID=1','-DARDUINO_USB_MODE=0','-DARDUINO_USB_CDC_ON_BOOT=1']
 def preprocess(f): return subprocess.run([compiler,'-E','-I'+str(t),*f,str(test)],capture_output=True)
 assert preprocess(flags).returncode==0
 for bad,expected in [
  ([f for f in flags if not f.startswith('-DUSB_as_HID')],b'requires USB_as_HID'),
  ([f.replace('MODE=0','MODE=1') for f in flags],b'requires ARDUINO_USB_MODE=0'),
  ([*flags,'-DLITE_VERSION=1'],b'requires the full Bruce'),
  ([f.replace('CDC_ON_BOOT=1','CDC_ON_BOOT=0') for f in flags],b'requires TinyUSB CDC')]:
  p=preprocess(bad); assert p.returncode!=0 and expected in p.stderr,p.stderr
 (t/'sdkconfig.h').write_text('#define CONFIG_TINYUSB_ENABLED 1\n#define CONFIG_TINYUSB_HID_ENABLED 0\n#define CONFIG_TINYUSB_CDC_ENABLED 1\n')
 p=preprocess(flags);assert p.returncode!=0 and b'requires USB OTG' in p.stderr
 print('PASS: real variant preprocesses successfully; missing native HID, wrong USB mode, disabled CDC/HID and Lite fail explicitly')

 # Compile and execute the actual Waveshare startup branch and actual cleanup.
 start=source.index('void ducky_startKb(')
 end=source.index('\n#endif',start)+len('\n#endif')
 startup=source[start:end]+'\n}\n'
 cleanup=function(source,'void ducky_endUsb(')
 harness=r'''
#include <cassert>
#include <cstdint>
#define WAVESHARE_28B 1
#define pgm_read_ptr(p) (*(p))
unsigned now=0; bool returnToMenu=false,cancel=false,mounted=true; int warning=0,error=0,status=0;
uint32_t millis(){return now;} void delay(int ms){now+=ms;}
int EscPress=1;bool check(int){return cancel;}
void displayError(const char*,bool){++error;}void displayWarning(const char*,bool){++warning;}
void printStatusBadUSBBLE(const char*){++status;}
const uint8_t map[128]={0};const uint8_t *keyboardLayouts[]={map};
struct {int badUSBBLEKeyboardLayout=0,badUSBBLEKeyDelay=20;} bruceConfig;
struct HIDInterface {int release=0,beginCount=0;void begin(const uint8_t*){++beginCount;}
void setDelay(int){}void releaseAll(){++release;}bool isConnected(){return mounted;}};
HIDInterface waveshareUsbKeyboard;
struct {bool ok=true;bool begin(){return ok;}} USB;
'''+cleanup+startup+r'''
int main(){
 HIDInterface *h=nullptr;
 for(int i=0;i<100;i++){ducky_startKb(h,false,0);assert(h==&waveshareUsbKeyboard&&!returnToMenu);ducky_endUsb(h);assert(h==nullptr);}
 assert(waveshareUsbKeyboard.beginCount==100&&waveshareUsbKeyboard.release==200);
 mounted=false;now=0;ducky_startKb(h,false,2);assert(h==nullptr&&returnToMenu&&now==15000&&warning==1);
 returnToMenu=false;cancel=true;now=0;ducky_startKb(h,false,0);assert(h==nullptr&&returnToMenu&&now==0&&warning==2);
 returnToMenu=false;cancel=false;USB.ok=false;ducky_startKb(h,false,0);assert(h==nullptr&&returnToMenu&&error==1);
 returnToMenu=false;USB.ok=true;mounted=true;ducky_startKb(h,false,0);assert(h==&waveshareUsbKeyboard&&!returnToMenu);
 ducky_endUsb(h);
}
'''
 cpp=t/'lifecycle.cpp';cpp.write_text(harness)
 exe=t/'lifecycle.exe'
 result=subprocess.run([compiler,'-std=c++11',str(cpp),'-o',str(exe)],capture_output=True)
 assert result.returncode==0,result.stderr.decode(errors='replace')
 subprocess.run([str(exe)],check=True)
print('PASS: actual USB startup/cleanup host harness: 100 reentries, timeout, cancel, USB failure, reconnect; mock host state, hardware untested')
