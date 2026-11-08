#pragma once
#include "AvatarCatalog.hpp"

namespace cards::avatar360::editor {
struct Tab {const char* name;std::array<int,16> fields;int count;};
inline constexpr std::array<Tab,7> tabs={{
    {"Headwear",{CHat},1},
    {"Hair",{CHair,CHairColor},2},
    {"Clothes",{CTop,CBottom,CShoes,COutfit},4},
    {"Accessories",{CGlasses,CGloves,CWrist,CEarrings,CRing},5},
    {"Face",{CEyes,CEyeColor,CBrows,CBrowColor,CMouth,CLipColor,CFacialHair,CFacialColor,CFacePaint,CFaceColor1,CFaceColor2,CShadow,CShadowColor,CNose,CChin,CEars},16},
    {"Body",{CBody,CSkin,CHeight,CBuild},4},
    {"Props",{CProp},1}
}};
inline constexpr int pageSize=8;
inline bool headGroup(int group){return group==0 || group==4 || group==6 || group==8 || (group>=10 && group<=18);}
inline Box tabBox(int tab){return {64.f+tab*165.f,54,160,48};}
inline Box tileBox(int tile){return {tile<4?200.f:936.f,208.f+(tile%4)*104.f,144,96};}
inline int count(const CatalogIndex& index,const Look& look,int field){
    const int group=fieldGroup(field),color=fieldColor(field);
    return group>=0?int(index.options(group,look.colors[0]).size()):color>=0?colorCount(color):0;
}
inline int selected(const CatalogIndex& index,const Look& look,int field){
    const int group=fieldGroup(field),color=fieldColor(field);
    if(group>=0){auto options=index.options(group,look.colors[0]);for(size_t i=0;i<options.size();++i)if((options[i]?options[i]->key:"")==look.items[group])return i;return 0;}
    return color>=0?look.colors[color]:0;
}
inline std::string name(const CatalogIndex& index,const Look& look,int field,int at){
    const int group=fieldGroup(field),color=fieldColor(field);
    if(group>=0){auto options=index.options(group,look.colors[0]);if(at<0 || at>=int(options.size()))return {};return options[at]?options[at]->name:headGroup(group)&&group>=16?"Default":"None";}
    if(color==0)return at?"Female":"Male";
    if(color==8){constexpr const char* labels[]={"Shortest","Short","Average","Tall","Tallest"};return labels[std::clamp(at,0,4)];}
    if(color==9){constexpr const char* labels[]={"Slimmest","Slim","Average","Broad","Broadest"};return labels[std::clamp(at,0,4)];}
    return "Colour "+std::to_string(at+1);
}
inline std::string thumbnail(const CatalogIndex& index,const Look& look,int field,int at){
    const int group=fieldGroup(field),color=fieldColor(field),body=look.colors[0];
    if(color==0)return "thumb-body-"+std::to_string(at)+".png";
    if(group<0)return {};
    auto options=index.options(group,body);if(at<0 || at>=int(options.size()))return {};
    return options[at]?"thumb-"+options[at]->key+"-"+std::to_string(body)+".png":std::string(headGroup(group)?"thumb-face-":"thumb-body-")+std::to_string(body)+".png";
}
struct Browser {
    int tab=0,option=0,focus=0;bool zoom=false;Look preview;
    int field()const{return tabs[tab].fields[option];}
    int page()const{return focus/pageSize;}
    int pages(const CatalogIndex& index,const Look& draft)const{return std::max(1,(count(index,draft,field())+pageSize-1)/pageSize);}
    void synchronize(const CatalogIndex& index,const Look& draft){focus=selected(index,draft,field());preview=draft;}
    void begin(const CatalogIndex& index,const Look& draft){tab=option=0;zoom=false;synchronize(index,draft);}
    void category(const CatalogIndex& index,const Look& draft,int delta){tab=(tab+delta%int(tabs.size())+int(tabs.size()))%int(tabs.size());option=0;synchronize(index,draft);}
    void setting(const CatalogIndex& index,const Look& draft,int delta){option=(option+delta%tabs[tab].count+tabs[tab].count)%tabs[tab].count;synchronize(index,draft);}
    void optionOrTab(const CatalogIndex& index,const Look& draft,int delta){if(tabs[tab].count>1)setting(index,draft,delta);else category(index,draft,delta);}
    void browse(const CatalogIndex& index,const Look& draft,int at){
        focus=std::clamp(at,0,std::max(0,count(index,draft,field())-1));preview=draft;
        const int delta=focus-selected(index,draft,field());index.cycle(preview,field(),delta);
    }
    void move(const CatalogIndex& index,const Look& draft,int horizontal,int vertical){browse(index,draft,focus+horizontal*4+vertical);}
    void turn(const CatalogIndex& index,const Look& draft,int delta){browse(index,draft,focus+delta*pageSize);}
    void use(const CatalogIndex& index,Look& draft){draft=preview;index.cycle(draft,field(),0);synchronize(index,draft);}
};
inline constexpr bool coversAllFields(){std::array<int,CFieldCount> seen{};for(const auto& tab:tabs)for(int i=0;i<tab.count;++i){if(tab.fields[i]<0 || tab.fields[i]>=CFieldCount)return false;++seen[tab.fields[i]];}for(int n:seen)if(n!=1)return false;return true;}
static_assert(coversAllFields(),"Every saved avatar customization must remain reachable exactly once");
}
