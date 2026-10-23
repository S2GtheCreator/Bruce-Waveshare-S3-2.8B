from pathlib import Path
import os,shutil,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
s=(root/'boards/waveshare_28b/badusb_pro.cpp').read_text(encoding='utf-8')
actual=s[s.index('bool readText('):s.index('bool safeSave(')]
stub=r'''
#include <string>
#include <map>
#include <sstream>
#include <type_traits>
#include <cstdint>
#include <cassert>
#include <iostream>
#include "pro_script_limits.h"
bool reserveOK=true,cancelRead=false;uint32_t now=0,step=0;
uint32_t millis(){uint32_t result=now;now+=step;return result;}void yield(){}enum{EscPress};bool check(int){return cancelRead;}
class String:public std::string{public:using std::string::string;String(){}String(const std::string&s):std::string(s){}
template<class T,typename std::enable_if<std::is_arithmetic<T>::value,int>::type=0> explicit String(T v){std::ostringstream n;n<<v;assign(n.str());}
bool reserve(size_t n){if(!reserveOK)return false;std::string::reserve(n);return true;}
};
void displayError(const String&,bool){}
const size_t MAX_SCRIPT=WS_SCRIPT_MAX_BYTES;const char*FILE_READ="r";
class FS;
struct File{FS*fs=nullptr;String path;size_t pos=0;bool good=false;operator bool()const{return good;}bool isDirectory();size_t size();int read();void close();};
struct FS{std::map<std::string,std::string>files;bool directory=false;int failAt=-1,reads=0,closed=0;File open(const String&p,const char*){File f;f.fs=this;f.path=p;f.good=files.count(p);return f;}};
bool File::isDirectory(){return fs->directory;}size_t File::size(){return good?fs->files[path].size():0;}
int File::read(){++fs->reads;if(!good||int(pos)==fs->failAt||pos>=size())return -1;return uint8_t(fs->files[path][pos++]);}
void File::close(){if(good)++fs->closed;good=false;}
'''
main=r'''
int main(){unsigned cases=0;String out;
auto reset=[](){reserveOK=true;cancelRead=false;now=0;step=0;};
{reset();FS f;f.files["/a"]="STRING TEST\n";assert(readText(f,"/a",out)&&out==f.files["/a"]&&f.closed==1);++cases;}
{reset();FS f;f.files["/a"]=std::string(512,'a')+"\r\n";assert(readText(f,"/a",out));++cases;}
{reset();FS f;f.files["/a"]=std::string(513,'a');assert(!readText(f,"/a",out));++cases;}
{reset();FS f;f.files["/a"]=std::string(129,'\n');assert(!readText(f,"/a",out));++cases;}
{reset();FS f;f.files["/a"]="STRING TEST\n";f.failAt=3;assert(!readText(f,"/a",out)&&f.reads==4&&f.closed==1);++cases;}
{reset();FS f;f.files["/a"]="TEST";reserveOK=false;assert(!readText(f,"/a",out)&&f.reads==0&&f.closed==1);++cases;}
{reset();FS f;f.files["/a"]="TEST";cancelRead=true;assert(!readText(f,"/a",out)&&f.reads==0&&f.closed==1);++cases;}
{reset();FS f;f.files["/a"]="TEST";step=6000;assert(!readText(f,"/a",out)&&f.reads==0&&f.closed==1);++cases;}
{reset();FS f;f.files["/a"]=std::string(16385,'a');assert(!readText(f,"/a",out)&&f.reads==0);++cases;}
{reset();FS f;assert(!readText(f,"/a",out)&&f.reads==0);++cases;}
{reset();FS f;f.files["/a"]="TEST";f.directory=true;assert(!readText(f,"/a",out)&&f.reads==0);++cases;}
std::cout<<"PASS: "<<cases<<" actual bounded read cases: success, CRLF, limits, I/O error, allocation failure, cancel, timeout, missing/directory\n";
}
'''
with tempfile.TemporaryDirectory() as d:
 t=Path(d);shutil.copy(root/'boards/waveshare_28b/pro_script_limits.h',t/'pro_script_limits.h')
 (t/'test.cpp').write_text(stub+actual+main,encoding='utf-8');exe=t/'test.exe'
 compiler=os.environ.get('HOST_CXX',shutil.which('g++')or'C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe')
 subprocess.run([compiler,'-std=c++11',str(t/'test.cpp'),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
