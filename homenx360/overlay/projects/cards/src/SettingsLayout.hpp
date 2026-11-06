#pragma once
#include <algorithm>
#include <array>
#include <vector>

namespace cards::settings360 {
enum Page {
    System, Console, Family, Memory, Network, Computers, Vision, Initial,
    Display, Audio, Language, Clock, Startup, AutoPlay, Remote, SystemInfo,
    Preferences, Look, Home, Lock, Notifications, Pictures, Updates, Count
};
inline constexpr std::array<const char*,Count> titles = {
    "System Settings", "Console Settings", "Family Settings", "Memory",
    "Network Settings", "Computers", "Xbox LIVE Vision", "Initial Setup",
    "Display", "Audio", "Language and Locale", "Clock", "Startup and Shutdown",
    "Auto-Play", "Remote Control", "System Info", "HomeNX Preferences",
    "Dashboard", "Games and Apps", "Lock Screen", "Notifications", "Pictures", "Updates"
};
struct Link { const char* label; const char* glyph; const char* detail; Page target; };
inline constexpr std::array<Link,8> systemLinks = {{
    {"Console Settings","power","Change your console settings, including display, audio, language, and remote control.",Console},
    {"Family Settings","friends","Choose profiles and configure HomeNX's lock screen. Nintendo parental restrictions are managed by Nintendo Switch Parental Controls.",Family},
    {"Memory","copy","View console memory and microSD card space, and move installed software.",Memory},
    {"Network Settings","wifi","Connect to your home network, view connection information, and test network settings.",Network},
    {"Computers","monitor","Connect this Switch to your computer with a USB cable to transfer files.",Computers},
    {"Xbox LIVE Vision","sys_mii","Xbox LIVE Vision requires an Xbox 360 camera. That peripheral and Xbox LIVE are unavailable on Nintendo Switch.",Vision},
    {"Initial Setup","pad","Choose a profile and review language, clock, network, and startup settings without resetting your console.",Initial},
    {"HomeNX Preferences","gear","Customize the dashboard, games, pictures, notifications, lock screen, and HomeNX tools.",Preferences}
}};
inline constexpr std::array<Link,8> consoleLinks = {{
    {"Display","monitor","Change TV output, handheld brightness, and display settings.",Display},
    {"Audio","speaker","Change dashboard music, menu sounds, volume, and Bluetooth audio devices.",Audio},
    {"Language and Locale","globe","Choose the language used by HomeNX. Console region and Nintendo Account settings remain managed by Nintendo.",Language},
    {"Clock","clock","Change clock display and, in the HOME frontend, console date and time.",Clock},
    {"Startup and Shutdown","power","Configure the original Xbox 360 startup animation and the Switch's sleep timers.",Startup},
    {"Auto-Play","play","Configure software launch preferences. Nintendo Switch does not support Xbox 360 disc auto-play.",AutoPlay},
    {"Remote Control","pad","Pair controllers, change button mapping, and configure Bluetooth and vibration.",Remote},
    {"System Info","dots","View this Switch's real firmware, Atmosphere version, storage, and HomeNX information.",SystemInfo}
}};
struct Rect {
    float x,y,w,h;
    bool contains(float px,float py) const { return px>=x && px<x+w && py>=y && py<y+h; }
};
inline constexpr Rect panel{194,110,890,484};
inline constexpr float rowX=209, rowY=125, rowW=423, pitch=46, rowH=46, listBottom=585;
inline constexpr int visibleRows=10;
inline Rect row(int index,float scroll=0,float slide=0) { return {rowX+slide,rowY+pitch*index-scroll,rowW,rowH}; }
inline int topRow(int selected,int count) {
    return std::clamp(selected-visibleRows+1,0,std::max(0,count-visibleRows));
}
inline int hitRow(float x,float y,int count,float scroll=0,float slide=0) {
    if (y<rowY || y>=listBottom || x<rowX+slide || x>=rowX+slide+rowW) return -1;
    const int index=static_cast<int>((y-rowY+scroll)/pitch);
    return index>=0 && index<count && row(index,scroll,slide).contains(x,y) ? index : -1;
}
struct Visit { int page,row; };
inline bool enter(int target,int& page,int& selected,std::vector<Visit>& history) {
    if(target<0 || target>=Count || target==page) return false;
    history.push_back({page,selected});page=target;selected=0;return true;
}
inline bool back(int& page,int& selected,std::vector<Visit>& history) {
    if(history.empty()) return false;
    const auto visit=history.back();history.pop_back();page=visit.page;selected=visit.row;return true;
}
inline int parent(int page) {
    if(page>=Display && page<=SystemInfo) return Console;
    if(page>=Look && page<=Updates) return Preferences;
    return System;
}
inline void seedHistory(int page,std::vector<Visit>& history) {
    history.clear();
    if(page==System) return;
    const int above=parent(page);
    history.push_back({System,above==Console?0:above==Preferences?7:std::clamp(page-1,0,7)});
    if(above!=System) history.push_back({above,page>=Look?page-Look:page-Display});
}
} // namespace cards::settings360
