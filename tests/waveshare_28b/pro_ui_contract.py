from pathlib import Path
import os,shutil
import subprocess,tempfile,shutil
root=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory() as d:
 t=Path(d)
 for name in ['scrollableTextArea.cpp','scrollableTextArea.h']:
  shutil.copy(root/'src/core'/name,t/name)
 shutil.copy(root/'lib/HAL/display/ws28b_text.h',t/'ws28b_text.h')
 (t/'display.h').write_text(r'''
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdint.h>
class String:public std::string { public:
 using std::string::string;
 String(){} String(const std::string&s):std::string(s){}
 bool isEmpty()const{return empty();}
 String substring(size_t a,size_t b=std::string::npos)const {return a>=size()?String():String(substr(a,b==std::string::npos?b:b-a));}
 bool endsWith(const char*s)const {size_t n=std::char_traits<char>::length(s);return size()>=n&&compare(size()-n,n,s)==0;}
 void remove(size_t a){erase(a);}
};
struct File { bool available(){return false;} String readStringUntil(char){return "";} int read(){return -1;} };
enum {SelPress,PrevPress,UpPress,NextPress,DownPress,EscPress};
inline bool check(int){return false;} inline void yield(){} inline void delay(int){}
const int FP=2,BORDER_PAD_X=10,BORDER_PAD_Y=28;
extern int tftWidth,tftHeight;
struct Config{int priColor=1,bgColor=0;};extern Config bruceConfig;
struct Display {
 int size=2,y=48,draws=0;std::vector<int> ys;
 void setTextColor(int){} void setTextSize(int s){size=s;} int getTextSize(){return size;}
 void fillRect(int,int,int,int,int){++draws;}
 int textWidth(const char*,int){return 15;} int fontHeight(){return 20;} int getCursorY(){return y;}
 void drawString(const String&,int,int y){ys.push_back(y);}
};extern Display tft;
inline void drawMainBorder(){} inline void printTitle(const String&){}
''')
 (t/'test.cpp').write_text(r'''
#include "scrollableTextArea.h"
#include "ws28b_text.h"
#include <cassert>
#include <iostream>
int tftWidth=480,tftHeight=640;Config bruceConfig;Display tft;
int main(){
 const char*letters=u8"çğıöşüÇĞİÖŞÜ";assert(wsTextCells(letters)==12);
 uint16_t cps[]={0xE7,0x11F,0x131,0xF6,0x15F,0xFC,0xC7,0x11E,0x130,0xD6,0x15E,0xDC};
 for(unsigned i=0;i<12;++i)assert(wsGlyph(cps[i])==128+i);
 for(size_t i=0;i<=12;++i){size_t bytes=wsTextBytes(letters,i);assert(bytes==2*i);assert(wsTextCells(std::string(letters,bytes).c_str())==i);}
 assert(wsGlyph(0x1F642)=='?');
 ScrollableTextArea empty;assert(empty.getLine(0).empty());empty.scrollToLine(999);empty.scrollDown();assert(empty.firstVisibleLine==0);
 ScrollableTextArea a(2,10,50,120,66,false,false);
 for(int i=0;i<20;++i)a.addLine("row");a.scrollDown();assert(a.firstVisibleLine==1);
 a.scrollToLine(999);assert(a.firstVisibleLine==17);a.scrollDown();assert(a.firstVisibleLine==17);
 assert(a.getLine(20).empty());a.draw();assert(tft.size==2);
 for(int y:tft.ys)assert(y>=50&&y<116);tft.ys.clear();
 a.clear();a.addLine(letters);assert(a.getMaxLines()==2);assert(wsTextCells(a.getLine(0).c_str())==8);assert(wsTextCells(a.getLine(1).c_str())==4);
 a.scrollToLine(999);assert(a.firstVisibleLine==0);a.draw();
 tft.size=3;a.draw(true);assert(tft.size==3);tft.size=2;
 a.clear();for(int i=0;i<600;++i)a.addLine("row");assert(a.getMaxLines()==512);assert(a.getLine(511).find("[view truncated")==0);
 a.clear();int before=tft.draws;a.draw();assert(tft.draws>before);
 std::cout<<"PASS: actual ScrollableTextArea bounds, one-step scroll, empty access, font restore, UTF-8 wrapping\n";
 std::cout<<"PASS: 12 Turkish display mappings, UTF-8 cell metrics and clipping boundaries\n";
}
''',encoding='utf-8')
 exe=t/'test.exe'
 cmd=[os.environ.get('HOST_CXX',shutil.which('g++') or 'C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe'),'-std=c++11','-DWS28B_TOUCH_PRO=1',str(t/'scrollableTextArea.cpp'),str(t/'test.cpp'),'-o',str(exe)]
 subprocess.run(cmd,check=True);subprocess.run([str(exe)],check=True)
import json,hashlib
manifest=json.loads((root/'boards/waveshare_28b/pro-preserved-source.json').read_text(encoding='utf-8'))
# THIRD_PARTY.md is deliberately extended for distribution compliance.
for rel,digest in manifest.items():
 if rel == 'THIRD_PARTY.md': continue
 assert hashlib.sha256((root/rel).read_bytes()).hexdigest()==digest,rel
print('PASS: original RGB/DMA, pin map, GT911, SD, USB CDC, partition and every HID library file byte preservation (distribution notices excluded)')
