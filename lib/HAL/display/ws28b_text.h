#pragma once
#include <stdint.h>
#include <stddef.h>
inline size_t wsTextCells(const char *s) { size_t n=0; for(;*s;++s) if((uint8_t(*s)&0xC0)!=0x80) ++n; return n; }
inline size_t wsTextBytes(const char *s,size_t cells) {size_t i=0,n=0;while(s[i]&&n<cells){++i;while(s[i]&&(uint8_t(s[i])&0xC0)==0x80)++i;++n;}return i;}
inline uint8_t wsGlyph(uint32_t cp) {
 const uint16_t codes[]={0xE7,0x11F,0x131,0xF6,0x15F,0xFC,0xC7,0x11E,0x130,0xD6,0x15E,0xDC};
 for(unsigned i=0;i<12;++i)if(cp==codes[i])return 0x80+i;
 return cp<128?uint8_t(cp):'?';
}
