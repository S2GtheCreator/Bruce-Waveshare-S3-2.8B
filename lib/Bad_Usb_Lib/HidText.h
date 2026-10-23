#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "HidTextTrMap.h"

enum class HidTextFailure : uint8_t { None, InvalidUtf8, Unsupported, FullReport };
struct HidTextError {
    HidTextFailure failure;
    uint32_t codepoint;
    size_t offset;
    HidTextError(HidTextFailure f=HidTextFailure::None,uint32_t cp=0,size_t at=0)
        : failure(f),codepoint(cp),offset(at) {}
};
struct HidTextStroke { uint8_t usage, modifiers; bool dead; };

inline void hidTextDescribe(const HidTextError &e, char *out, size_t size) {
    if (e.failure == HidTextFailure::InvalidUtf8)
        snprintf(out,size,"Invalid UTF-8 at byte %lu",(unsigned long)e.offset);
    else if (e.failure == HidTextFailure::Unsupported)
        snprintf(out,size,"Unsupported U+%04lX at byte %lu",(unsigned long)e.codepoint,(unsigned long)e.offset);
    else if (e.failure == HidTextFailure::FullReport)
        snprintf(out,size,"HID key report full at byte %lu",(unsigned long)e.offset);
    else if (size) out[0]=0;
}

// Strict RFC 3629 decoding: no byte can become a modifier/function key.
inline bool hidTextDecode(const uint8_t *data,size_t size,size_t &pos,uint32_t &cp) {
    if (pos>=size) return false;
    uint8_t lead=data[pos++];
    if (lead<0x80) { cp=lead; return true; }
    unsigned extra; uint32_t minimum;
    if (lead>=0xC2 && lead<=0xDF) {extra=1;cp=lead&31;minimum=0x80;}
    else if (lead>=0xE0 && lead<=0xEF) {extra=2;cp=lead&15;minimum=0x800;}
    else if (lead>=0xF0 && lead<=0xF4) {extra=3;cp=lead&7;minimum=0x10000;}
    else return false;
    if (extra>size-pos) return false;
    for(unsigned i=0;i<extra;++i) {
        uint8_t next=data[pos++]; if((next&0xC0)!=0x80) return false;
        cp=(cp<<6)|(next&63);
    }
    return cp>=minimum && cp<=0x10FFFF && !(cp>=0xD800 && cp<=0xDFFF);
}

inline bool hidTextMap(uint32_t cp,const uint8_t *ascii,bool turkish,bool caps,HidTextStroke &s) {
    s={0,0,false};
    if (cp<128) {
        uint8_t packed=(turkish && caps) ? hidTextTrCapsAscii[cp] : ascii[cp];
        if(!packed) return false;
        s.usage=packed&63;
        s.modifiers=((packed&128)?2:0)|((packed&64)?64:0);
        if(s.usage==0x32) s.usage=0x64; // ISO non-US backslash
        if(!turkish && caps && ((cp>='A'&&cp<='Z')||(cp>='a'&&cp<='z'))) s.modifiers^=2;
        s.dead=turkish && (cp=='^'||cp=='`'||cp=='~');
        return true;
    }
    if(turkish) for(const auto &e:hidTextTrUnicode) if(e.codepoint==cp) {
        s=caps ? HidTextStroke{e.capsUsage,e.capsModifiers,e.capsDead} : HidTextStroke{e.usage,e.modifiers,e.dead};
        return s.usage!=0;
    }
    return false;
}

// Preflight the entire Print buffer before transmitting any of it.
template<class Emit>
size_t hidTextWrite(const uint8_t *data,size_t size,const uint8_t *ascii,bool turkish,bool caps,
                    Emit emit,HidTextError &error) {
    error={};
    if (size && !data) {error={HidTextFailure::InvalidUtf8,0,0};return 0;}
    for(size_t pos=0;pos<size;) {
        size_t start=pos; uint32_t cp=0; HidTextStroke stroke;
        if(!hidTextDecode(data,size,pos,cp)) {error={HidTextFailure::InvalidUtf8,0,start};return 0;}
        if(cp==13) continue; // Preserve Bruce Print CRLF semantics.
        if(!hidTextMap(cp,ascii,turkish,caps,stroke)) {error={HidTextFailure::Unsupported,cp,start};return 0;}
    }
    for(size_t pos=0;pos<size;) {
        size_t start=pos;uint32_t cp=0;HidTextStroke stroke;
        hidTextDecode(data,size,pos,cp);if(cp==13)continue;
        hidTextMap(cp,ascii,turkish,caps,stroke);
        if(!emit(stroke) || (stroke.dead && !emit(HidTextStroke{0x2C,0,false}))) {
            error={HidTextFailure::FullReport,cp,start};return start;
        }
    }
    return size;
}

// Preserve existing held keys/modifiers across a text character.
template<class Report,class Send,class Wait>
bool hidTextEmit(const HidTextStroke &s,const Report &saved,Send send,Wait wait) {
    Report frame=saved;frame.modifiers|=s.modifiers;
    unsigned slot=0;for(;slot<6 && frame.keys[slot];++slot) {}
    if(slot==6) return false;
    frame.keys[slot]=s.usage;send(frame);wait();send(saved);wait();return true;
}

inline bool hidTextCommandChord(uint8_t c,uint8_t modifiers) {
    return (modifiers&0x9D) && ((c>='A'&&c<='Z')||(c>='a'&&c<='z'));
}
