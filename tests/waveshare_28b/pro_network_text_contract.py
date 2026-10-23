from pathlib import Path
import os,shutil,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory() as d:
 t=Path(d)
 shutil.copy(root/'boards/waveshare_28b/pro_network_text.h',t/'pro_network_text.h')
 for f in ['HidText.h','HidTextTrMap.h']:shutil.copy(root/'lib/Bad_Usb_Lib'/f,t/f)
 (t/'Arduino.h').write_text(r'''
#pragma once
#include <string>
#include <stdint.h>
#include <cstdio>
class String:public std::string{public:using std::string::string;String(){}String(const std::string&s):std::string(s){}
bool isEmpty()const{return empty();}bool reserve(size_t n){std::string::reserve(n);return true;}
int indexOf(char c)const{size_t n=find(c);return n==npos?-1:int(n);}
String substring(size_t a,size_t b=npos)const{return a>=size()?String():String(substr(a,b==npos?b:b-a));}
void replace(const char*a,const char*b){size_t p=0;while((p=find(a,p))!=npos){std::string::replace(p,std::char_traits<char>::length(a),b);p+=std::char_traits<char>::length(b);}}
};
''')
 (t/'test.cpp').write_text(r'''
#include "pro_network_text.h"
#include <cassert>
#include <iostream>
int main(){unsigned cases=0;
auto label=[&](const String&s,const String&e){assert(wsNetworkLabel(s)==e);++cases;};
auto csv=[&](const String&s,const String&e){assert(wsCsvField(s)==e);++cases;};
label("normal","normal");label("line\nnext","line\\x0Anext");label("a\x1B" "b","a\\x1Bb");
label("a\\b","a\\x5Cb");label(String("\xFF=A",3),"\\xFF=A");label(u8"çğıöşüÇĞİÖŞÜ",u8"çğıöşüÇĞİÖŞÜ");
csv("=1+1","\"'=1+1\"");csv("  =SUM(1)","\"'  =SUM(1)\"");csv("a\"b","\"a\"\"b\"");
csv("\n=1","\"\\x0A=1\"");csv("","\"\"");
std::cout<<"PASS: "<<cases<<" actual SSID/CSV helper cases: controls, malformed UTF-8, Turkish, quoting and formula prefixes\n";
}
''',encoding='utf-8')
 exe=t/'test.exe';compiler=os.environ.get('HOST_CXX',shutil.which('g++')or'C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe')
 subprocess.run([compiler,'-std=c++11','-I'+str(t),str(t/'test.cpp'),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
