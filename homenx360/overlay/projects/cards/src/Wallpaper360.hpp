#pragma once
#include <array>
#include <string>
#include <string_view>

namespace cards::wallpaper360 {
struct Entry { const char* id; const char* name; const char* file; };
inline constexpr std::array<Entry,11> entries={{
    {"night","NXE Night","night.png"},
    {"rings-green","NXE Rings - Green","rings-green.png"},
    {"rings-orange","NXE Rings - Orange","rings-orange.png"},
    {"rings-blue","NXE Rings - Blue","rings-blue.png"},
    {"rings-purple","NXE Rings - Purple","rings-purple.png"},
    {"halo3-mythic","Halo 3 - Mythic","halo3-mythic.jpg"},
    {"halo3-splatter","Halo 3 - Splatter Spree","halo3-splatter.jpg"},
    {"gears2-beauty","Gears 2 - Destroyed Beauty","gears2-beauty.jpg"},
    {"banjo-kazooie","Banjo-Kazooie","banjo-kazooie.jpg"},
    {"fable2-albion","Fable II - Albion","fable2-albion.jpg"},
    {"winter-holiday","Winter Holiday","winter-holiday.jpg"},
}};
inline std::string selection(const Entry& entry){return std::string("nxe:")+entry.id;}
inline const Entry* find(std::string_view saved){
    for(const auto& entry:entries)if(saved==selection(entry))return &entry;
    return nullptr;
}
// Persist IDs rather than romfs:/ paths so the same settings work in app and HOME.
inline std::string resolve(const std::string& saved,const std::string& assets){
    if(const auto* entry=find(saved))return assets+"wallpapers/"+entry->file;
    return saved.rfind("nxe:",0)==0?std::string():saved;
}
inline std::string label(const std::string& saved){
    if(saved.empty())return "Classic NXE";
    if(const auto* entry=find(saved))return entry->name;
    return "Custom picture";
}
static_assert(entries.size()+1<=12,"The theme picker must fit its twelve preview tiles");
}
