#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

namespace cards::theme360 {
struct Color {float r,g,b,a=1;};
struct Palette {const char* name;Color color;};
inline constexpr std::array<Palette,7> palettes={{
    {"White",{.92f,.92f,.92f,1}}, {"Blue",{.36f,.68f,1,1}},
    {"Green",{.35f,.85f,.06f,1}}, {"Pink",{1,.46f,.72f,1}},
    {"Orange",{1,.63f,.26f,1}}, {"Purple",{.72f,.55f,1,1}},
    {"Gold",{1,.84f,.36f,1}}
}};
inline constexpr int defaultAccent=2;
inline int normalize(int accent){return accent>=0 && accent<int(palettes.size())?accent:defaultAccent;}
inline Color accent(int selected,float alpha=1){auto color=palettes[normalize(selected)].color;color.a=alpha;return color;}
// Keep the authored NXE green shading for the Green palette. Other palettes
// replace its hue while preserving lightness and opacity; White is grayscale.
inline Color tint(Color source,int selected){
    selected=normalize(selected);if(selected==defaultAccent)return source;
    const auto seed=accent(selected);const float hi=std::max({seed.r,seed.g,seed.b}),lo=std::min({seed.r,seed.g,seed.b}),range=hi-lo;
    float hue=0,saturation=0;
    if(range>1e-6f){
        if(hi==seed.r)hue=std::fmod((seed.g-seed.b)/range+6.f,6.f);
        else if(hi==seed.g)hue=(seed.b-seed.r)/range+2.f;
        else hue=(seed.r-seed.g)/range+4.f;
        saturation=range/(1-std::abs(hi+lo-1));
    }
    const float srcHi=std::max({source.r,source.g,source.b}),srcLo=std::min({source.r,source.g,source.b});
    const float light=(srcHi+srcLo)*.5f,den=1-std::abs(2*light-1);
    const float sat=den>1e-6f?(srcHi-srcLo)/den*saturation:0;
    const float c=(1-std::abs(2*light-1))*sat,x=c*(1-std::abs(std::fmod(hue,2.f)-1)),m=light-c*.5f;
    Color out{0,0,0,source.a};
    if(hue<1)out={c,x,0,source.a};else if(hue<2)out={x,c,0,source.a};
    else if(hue<3)out={0,c,x,source.a};else if(hue<4)out={0,x,c,source.a};
    else if(hue<5)out={x,0,c,source.a};else out={c,0,x,source.a};
    out.r=std::clamp(out.r+m,0.f,1.f);out.g=std::clamp(out.g+m,0.f,1.f);out.b=std::clamp(out.b+m,0.f,1.f);return out;
}
struct Values {int accent=defaultAccent;std::string wallpaper;bool operator==(const Values&)const=default;};
inline std::string userId(uint64_t lo,uint64_t hi){
    if(!lo && !hi)return {};char id[33];std::snprintf(id,sizeof(id),"%016llX%016llX",static_cast<unsigned long long>(lo),static_cast<unsigned long long>(hi));return id;
}
}
