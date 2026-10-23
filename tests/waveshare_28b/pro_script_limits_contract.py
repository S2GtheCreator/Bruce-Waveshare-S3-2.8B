from pathlib import Path
import os,shutil,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory() as d:
 t=Path(d);shutil.copy(root/'boards/waveshare_28b/pro_script_limits.h',t/'pro_script_limits.h')
 (t/'test.cpp').write_text(r'''
#include "pro_script_limits.h"
#include <cassert>
#include <string>
#include <iostream>
int main(){unsigned cases=0;size_t line=0;
auto check=[&](const std::string&s,WsScriptLimit expected){assert(wsScriptValidate(s.data(),s.size(),line)==expected);++cases;};
check("",WsScriptLimit::None);check("ENTER\n",WsScriptLimit::None);check(std::string(128,'\n'),WsScriptLimit::None);
check(std::string(129,'\n'),WsScriptLimit::Lines);check(std::string(16384,'\n'),WsScriptLimit::Lines);
check(std::string(512,'a'),WsScriptLimit::None);check(std::string(513,'a'),WsScriptLimit::LineBytes);
check(std::string(512,'a')+"\r\n",WsScriptLimit::None);check(std::string(16385,'a'),WsScriptLimit::Bytes);
std::string tr;for(int i=0;i<256;++i)tr+=u8"ç";check(tr,WsScriptLimit::None);check(tr+u8"ç",WsScriptLimit::LineBytes);
std::string exact;for(int i=0;i<128;++i)exact+=std::string(127,'a')+"\n";assert(exact.size()==16384);check(exact,WsScriptLimit::None);
assert(wsScriptValidate(nullptr,1,line)==WsScriptLimit::Bytes);++cases;
const std::string doc="STRING TEST\r\nENTER";
assert(wsScriptLineMatches(doc.data(),doc.size(),0,13,"STRING TEST\r",12));++cases;
assert(wsScriptLineMatches(doc.data(),doc.size(),13,18,"ENTER",5));++cases;
assert(!wsScriptLineMatches(doc.data(),doc.size(),0,7,"STRING",6));++cases;
assert(!wsScriptLineMatches(doc.data(),doc.size(),0,13,"STRING FAIL\r",12));++cases;
assert(!wsScriptLineMatches(doc.data(),doc.size(),0,12,"STRING TEST\r",12));++cases;
assert(!wsScriptLineMatches(doc.data(),doc.size(),18,18,"",0));++cases;
assert(wsScriptLineMatches("\n",1,0,1,"",0));++cases;
std::cout<<"PASS: "<<cases<<" actual pre-split script limits: 16 KiB, 128 lines, 512 UTF-8 bytes/line, CRLF and newline-flood bounds\n";
}
''',encoding='utf-8')
 exe=t/'test.exe';compiler=os.environ.get('HOST_CXX',shutil.which('g++')or'C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe')
 subprocess.run([compiler,'-std=c++11',str(t/'test.cpp'),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
