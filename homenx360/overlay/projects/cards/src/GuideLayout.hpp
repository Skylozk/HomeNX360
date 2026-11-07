#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace cards::guide360 {
struct Box {
    float x,y,w,h;
    bool contains(float px,float py) const { return px>=x && px<x+w && py>=y && py<y+h; }
};
enum Tab {Marketplace,Games,Profile,Friends,Media,Settings,Count};
inline constexpr std::array<const char*,Count> names={"Marketplace","Games","Profile","Friends","Media","Settings"};
inline constexpr float left=271.f,width=738.f,right=left+width;
inline int wrap(int tab) { return (tab%Count+Count)%Count; }
inline Box blade(int index,int selected) {
    const float x=left+index*38.f+(index>selected?510.f:0.f);
    const float inset=6.f*std::abs(index-selected);
    return {x,200.f+inset,index==selected?548.f:38.f,316.f-2.f*inset};
}
inline Box row(int index,int selected) { return {blade(selected,selected).x+59.f,212.f+index*43.f,472.f,41.f}; }
inline int hitBlade(float x,float y,int selected) {
    for(int i=0;i<Count;++i) if(i!=selected && blade(i,selected).contains(x,y)) return i;
    return -1;
}
inline int hitRow(float x,float y,int selected,int count) {
    for(int i=0;i<count;++i) if(row(i,selected).contains(x,y)) return i;
    return -1;
}
} // namespace cards::guide360
