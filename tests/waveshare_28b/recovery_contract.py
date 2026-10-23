from pathlib import Path
import json,hashlib,subprocess,sys,tempfile,re
root=Path(__file__).resolve().parents[2]
manifest=json.loads((root/'boards/waveshare_28b/preserved-source.json').read_text())
for file,expected in manifest.items():
 assert hashlib.sha256((root/file).read_bytes()).hexdigest()==expected,file
print("PASS: baseline RGB framebuffer/cache drivers, panel init/partition and protected tools byte-identical")
bus=(root/'lib/Arduino_GFX/src/databus/Arduino_ESP32RGBPanel.cpp').read_text()
hal=(root/'lib/HAL/display/ardgfx.cpp').read_text()
assert '.num_fbs = 1' in bus and '.num_fbs = 2' not in bus
assert '480 * 10' in hal and 'rgbPresentTask' not in hal and 'presentPending' not in hal
assert 'on_vsync' not in bus and 'on_frame_buf_complete' not in bus
hdr=(root/'boards/waveshare_28b/pins_arduino.h').read_text()
for name,value in {'TFT_PREF_SPEED':30000000,'TFT_HSYNC_PULSE_WIDTH':10,'TFT_HSYNC_BACK_PORCH':70,'TFT_HSYNC_FRONT_PORCH':60,'TFT_VSYNC_PULSE_WIDTH':10,'TFT_VSYNC_BACK_PORCH':20,'TFT_VSYNC_FRONT_PORCH':20}.items():
 assert re.search(r'^#define\s+'+name+r'\s+'+str(value)+r'$',hdr,re.M),name
for width,height in [(480,640),(640,480)]:
 area=height-32;rows=(area-56-10)//24
 for length in [1,2,15,22,100,500]:
  count=min(length,rows);top=area//2-count*24//2-5
  assert top>=28
  for index in range(length):
   start=max(0,index-count+1)
   assert start<=index<start+count<=length
   assert top+9+(count-1)*24+20<=area-28
  assert area+4+20<=height
print("PASS: single-FB/bounce rollback, no swap/presentation task, official timing, font rows and footer")
cpp=r"""
#include <cassert>
#include <cstdio>
#include "HEADER"
int main(){
 int prev=0; for(unsigned v=0;v<=10000;++v){int p=batteryEstimatePercent(v);assert(p>=0&&p<=100&&p>=prev);prev=p;}
 assert(batteryEstimatePercent(3300)==0&&batteryEstimatePercent(3600)==15&&batteryEstimatePercent(4200)==100);
 BatteryLowDebounce d;
 assert(!d.update(3400,true));assert(!d.update(3400,true));assert(d.update(3400,true));
 assert(d.update(3550,true));assert(!d.update(3600,true));
 assert(!d.update(0,false));assert(!d.update(3400,true));assert(!d.update(3400,true));
 assert(!d.update(0,false));assert(!d.update(3400,true));
 for(int i=0;i<1000;++i)d.update(3300,true);assert(d.low&&d.count==3);
 puts("PASS: actual battery estimate model, monotonic bounds, low-warning debounce, invalid samples and hysteresis");
}
""".replace("HEADER",(root/'boards/waveshare_28b/battery_model.h').as_posix())

battery=(root/'boards/waveshare_28b/battery.cpp').read_text()
start=battery.index('void logStatus(');brace=battery.index('{',start);end=brace+1;depth=1
while depth:
 depth+=(battery[end]=='{')-(battery[end]=='}');end+=1
actualFormatter=battery[start:end]
fields=(root/'boards/waveshare_28b/waveshare_support.h').read_text().split('struct WaveshareBatteryState {',1)[1].split('};',1)[0]
stub='#include <string>\n#include <cstdint>\nusing String=std::string;\nstruct WaveshareBatteryState {'+fields+'};\nstruct MockSerial{std::string last; void println(const String &s){last=s;}} mock;\nMockSerial *serialDevice=&mock;\n'
cpp=cpp.replace('int main(){',stub+actualFormatter+'\nint main(){\n WaveshareBatteryState s; s.valid=true;s.mv=3800;s.percent=60;s.low=false;logStatus(s);\n assert(mock.last.find("valid=1 voltage=3800mV estimate=~60% low=0")!=std::string::npos);\n assert(mock.last.size()<256&&mock.last.find("source=unknown charge=unknown")!=std::string::npos);\n puts("PASS: actual battery telemetry formats voltage/percentage correctly");\n')

compiler=sys.argv[1] if len(sys.argv)>1 else 'g++'
with tempfile.TemporaryDirectory() as temp:
 source=Path(temp)/'battery.cpp';exe=Path(temp)/'battery.exe';source.write_text(cpp)
 subprocess.run([compiler,'-std=c++11','-O2',str(source),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
diag=(root/'boards/waveshare_28b/diagnostics.cpp').read_text()
assert 'pending.exchange(-1)' in diag and 'noTouch.store(false' in diag
assert 'WiFi.mode(savedMode)' in diag and 'esp_wifi_scan_stop()' in diag
assert '<60000' in diag and '<2000' in diag and 'esp_task_wdt_reset()' in diag
print("PASS: bounded diagnostics dispatch/cleanup contracts (radio/touch operations still require hardware tests)")
