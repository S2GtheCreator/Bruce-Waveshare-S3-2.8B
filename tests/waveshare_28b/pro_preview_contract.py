from pathlib import Path
import os,shutil
import subprocess,tempfile,re,shutil
root=Path(__file__).resolve().parents[2]
s=(root/'boards/waveshare_28b/badusb_pro.cpp').read_text(encoding='utf-8')
start=s.index('        auto lines = split(doc);',s.index('void preview('));end=s.index('        String info =',start)
actual=s[start:end]
ducky=(root/'src/modules/badusb_ble/ducky_typer.cpp').read_text(encoding='utf-8')
table=ducky[ducky.index('const DuckyCommandLookup duckyCmds[]'):];table=table[:table.index('\n};')]
names=re.findall(r'\{"([^"\n]+)"',table)
stub=r'''
#include <string>
#include <vector>
#include <sstream>
#include <type_traits>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <cstdint>
#include "HidText.h"
class String:public std::string{public:using std::string::string;String(){}String(const std::string&s):std::string(s){}
template<class T,typename std::enable_if<std::is_arithmetic<T>::value,int>::type=0> explicit String(T n){std::ostringstream v;v<<n;assign(v.str());}
bool isEmpty()const{return empty();}int indexOf(char c)const{auto p=find(c);return p==npos?-1:int(p);}
bool startsWith(const char*s)const{return find(s)==0;}bool endsWith(const char*s)const{size_t n=std::char_traits<char>::length(s);return size()>=n&&compare(size()-n,n,s)==0;}
String substring(size_t a,size_t b=npos)const{return a>=size()?String():String(substr(a,b==npos?b:b-a));}
void remove(size_t a){erase(a);}
};
struct Config{uint16_t badUSBBLEKeyDelay=10;int badUSBBLEKeyboardLayout=13;}bruceConfig;
std::vector<String>split(const String&s){std::vector<String>v;size_t a=0;for(size_t i=0;i<s.length();++i)if(s[i]=='\n'){v.push_back(s.substring(a,i));a=i+1;}if(a<s.length()||v.empty())v.push_back(s.substring(a));return v;}
void*findDuckyCombination(const char*){return nullptr;}
struct Result{uint64_t ms;size_t errors;bool unbounded;};
'''
with tempfile.TemporaryDirectory() as d:
 t=Path(d)
 for name in ['HidText.h','HidTextTrMap.h']:shutil.copy(root/'lib/Bad_Usb_Lib'/name,t/name)
 tables='\n#define PROGMEM\n#define SHIFT 0x80\n#define ALT_GR 0x40\n'
 for name in ['KeyboardLayout_en_US.cpp','KeyboardLayout_tr_TR.cpp']:
  tables+=(root/'lib/Bad_Usb_Lib'/name).read_text(encoding='utf-8').replace('#include "KeyboardLayout.h"','')+'\n'
 lookup='void*findDuckyCommand(const char*c){const char*names[]={'+','.join('"'+v+'"' for v in names)+'};for(const char*n:names)if(std::string(n)==c)return (void*)1;return nullptr;}\n'
 main=r'''
int main(){unsigned cases=0;
auto good=[&](const char*s,uint64_t ms){auto r=preview(s);assert(r.errors==0&&r.ms==ms);++cases;};
auto bad=[&](const char*s){assert(preview(s).errors>0);++cases;};
good("REM test\nDELAY 2000\nSTRING TEST\nENTER\n",2080);
good("DELAY 100\nREPEAT 3\n",400);
good("STRING_DELAY 50\nSTRING AB\nREPEAT 2\n",280);
good("DEFAULT_STRING_DELAY 20\nSTRINGLN A\nREPEAT 2\n",240);
good("STRING ^\n",40);good("STRINGLN ^\n",60);
good(u8"STRINGLN çğıöşüÇĞİÖŞÜ\n",260);
good("DEFAULTDELAY\nREPEAT 2\n",300);
assert(preview("WAIT_FOR_BUTTON_PRESS\nDELAY 500\n").unbounded);++cases;
bad("REPEAT 2\n");bad("DELAY -1\n");bad("DELAY 0\n");bad("DELAY 4294967296\n");
bad("STRING test\nREPEAT 65536\n");bad("STRING_DELAY 1000\nSTRING test\n");
bad(u8"GUI ç\n");bad(u8"STRING prefix🙂suffix\n");bad("UNKNOWN abc\n");
bad("ALTSTRING test\n");
bruceConfig.badUSBBLEKeyDelay=300;bad("STRING test\n");bruceConfig.badUSBBLEKeyDelay=10;
bruceConfig.badUSBBLEKeyboardLayout=0;bad(u8"STRING ç\n");good("STRING TEST\n",80);
std::cout<<"PASS: "<<cases<<" actual preview dry-run cases: nominal timing, repeats, dead keys, TR/US, invalid arguments and unsupported text\n";
}
'''
 code=stub+tables+lookup+'Result preview(const String &doc){\n'+actual+'\nreturn {delayMs,unsupported,unboundedWait};}\n'+main
 (t/'test.cpp').write_text(code,encoding='utf-8');exe=t/'test.exe'
 subprocess.run([os.environ.get('HOST_CXX',shutil.which('g++') or 'C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe'),'-std=c++11',str(t/'test.cpp'),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
