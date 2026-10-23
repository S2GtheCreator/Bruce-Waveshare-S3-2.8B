from pathlib import Path
import os,shutil
import subprocess,tempfile,shutil
root=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory() as d:
 t=Path(d);shutil.copy(root/'boards/waveshare_28b/battery_model.h',t/'battery_model.h')
 (t/'test.cpp').write_text(r'''
#include "battery_model.h"
#include <cassert>
#include <iostream>
int main(){
 assert(batteryEstimatePercent(0)==0);assert(batteryEstimatePercent(3300)==0);assert(batteryEstimatePercent(4200)==100);assert(batteryEstimatePercent(5000)==100);
 int previous=0;for(unsigned v=2500;v<=4500;++v){int p=batteryEstimatePercent(v);assert(p>=previous&&p>=0&&p<=100);previous=p;}
 BatteryLowDebounce a;assert(!a.update(3400,true));assert(!a.update(3400,true));assert(a.update(3400,true));assert(a.update(3550,true));assert(!a.update(3600,true));
 BatteryLowDebounce b;assert(!b.update(3400,true));assert(!b.update(3400,true));assert(!b.update(0,false));assert(!b.update(3400,true));assert(!b.update(3400,true));assert(b.update(3400,true));
 std::cout<<"PASS: actual battery model monotonic 2500..4500 mV, saturation, 3-sample low warning, hysteresis and invalid-sample reset\n";
}
''')
 exe=t/'test.exe';subprocess.run([os.environ.get('HOST_CXX',shutil.which('g++') or 'C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe'),'-std=c++11',str(t/'test.cpp'),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
