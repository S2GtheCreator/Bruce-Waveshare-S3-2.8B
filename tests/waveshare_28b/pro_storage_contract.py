from pathlib import Path
import os,shutil
import subprocess,tempfile
src=(Path(__file__).resolve().parents[2]/'boards/waveshare_28b/badusb_pro.cpp').read_text(encoding='utf-8')
function=src[src.index('bool safeSave('):src.index('struct Manager {')]
stub=r'''
#include <string>
#include <map>
#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstring>
class String:public std::string {public: using std::string::string;String(){}String(const std::string&s):std::string(s){} };
const char*FILE_READ="r";const char*FILE_WRITE="w";
void displayError(const String&,bool){}void displayWarning(const String&,bool){}
class FS;
struct File{FS*fs=nullptr;String name;size_t pos=0;bool good=false;operator bool()const{return good;}
size_t write(const uint8_t*,size_t);void flush(){}void close(){good=false;}size_t size();int read();};
struct FS{std::map<std::string,std::string> files;bool shortWrite=false,corrupt=false;int failRename=0,renames=0;
bool exists(const String&p){return files.count(p);}
File open(const String&p,const char*mode){File f;f.fs=this;f.name=p;if(mode==FILE_WRITE){files[p]="";f.good=true;}else f.good=exists(p);return f;}
bool rename(const String&a,const String&b){++renames;if(failRename==renames||!exists(a)||exists(b))return false;files[b]=files[a];files.erase(a);return true;}
bool remove(const String&p){return files.erase(p)>0;}};
size_t File::write(const uint8_t*d,size_t n){if(fs->shortWrite&&n)--n;fs->files[name].append((const char*)d,n);if(fs->corrupt&&n)fs->files[name][0]^=1;return n;}
size_t File::size(){return good?fs->files[name].size():0;}
int File::read(){if(!good||pos>=size())return -1;return uint8_t(fs->files[name][pos++]);}
'''
test=r'''
int main(){
 {FS f;assert(safeSave(f,"/a.txt","new"));assert(f.files["/a.txt"]=="new");assert(f.files.size()==1);}
 {FS f;f.files["/a.txt"]="old";assert(safeSave(f,"/a.txt","new"));assert(f.files["/a.txt"]=="new");assert(f.files.size()==1);}
 {FS f;f.files["/a.txt"]="old";f.shortWrite=true;assert(!safeSave(f,"/a.txt","new"));assert(f.files["/a.txt"]=="old");assert(f.files.size()==1);}
 {FS f;f.files["/a.txt"]="old";f.corrupt=true;assert(!safeSave(f,"/a.txt","new"));assert(f.files["/a.txt"]=="old");}
 {FS f;f.files["/a.txt"]="old";f.failRename=1;assert(!safeSave(f,"/a.txt","new"));assert(f.files["/a.txt"]=="old");}
 {FS f;f.files["/a.txt"]="old";f.failRename=2;assert(!safeSave(f,"/a.txt","new"));assert(f.files["/a.txt"]=="old");assert(f.files["/a.txt.pro-tmp"]=="new");}
 {FS f;f.files["/a.txt"]="old";f.files["/a.txt.pro-bak"]="recovery";assert(!safeSave(f,"/a.txt","new"));assert(f.files["/a.txt"]=="old");assert(f.files["/a.txt.pro-bak"]=="recovery");}
 {FS f;f.files["/a.txt.pro-tmp"]="pending";assert(!safeSave(f,"/a.txt","new"));assert(f.files["/a.txt.pro-tmp"]=="pending");}
 std::cout<<"PASS: actual safeSave algorithm: 8 create/replace/short-write/corruption/rename/rollback/stale-recovery cases\n";
}
'''
with tempfile.TemporaryDirectory() as d:
 t=Path(d);(t/'test.cpp').write_text(stub+function+test);exe=t/'test.exe'
 subprocess.run([os.environ.get('HOST_CXX',shutil.which('g++') or 'C:/Program Files (x86)/TDM-GCC-32/bin/g++.exe'),'-std=c++11',str(t/'test.cpp'),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
