#include "../projects/cards/src/SettingsLayout.hpp"
#include "../projects/cards/src/NxeLayout.hpp"
#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace nxui {
enum class Button {DUp,DDown,DLeft,DRight,LStickU,LStickD,LStickL,LStickR,
    RStickU,RStickD,RStickL,RStickR,A,B,Plus,Count};
struct Input {
    std::array<bool,int(Button::Count)> down{},held{};
    bool isDown(Button b)const{return down[int(b)];}
    bool isHeld(Button b)const{return held[int(b)];}
    void press(Button b){down.fill(false);held.fill(false);down[int(b)]=true;}
};
}

// Compile the production page builder with a host callback fixture. No console
// services are mocked as successful: this checks routing and callback ownership.
namespace cards {
class Audio; // Production has this class; the page enum must stay qualified.
inline float brightnessWritten=-1;
inline void setBrightness(float value){brightnessWritten=value;}
class CardsActivity {
public:
    enum Page { kGames,kHomebrew,kNintendo,kMedia,kPageCount };
    struct MenuItem {
        enum Kind {Action,Toggle,Choice,Slider,Info};
        Kind kind=Action;std::string label,detail;const char* glyph=nullptr;
        std::function<void()> run;
        std::function<std::string()> value;
        std::function<void(int)> change;
        std::function<float()> level;
    };
    struct Section {std::string name;const char* icon;std::vector<MenuItem> rows;};
    std::vector<Section> m_sheet;
    struct {float brightness=.5f;} m_quick;
    bool m_sheetOpen=true,m_sheetClosing=false,m_sheetOnRows=true;
    int m_sheetSec=0,m_sheetRow=0;
    float m_sheetFocus=0,m_sheetScroll=0,m_sheetDirection=1,m_sheetRowsIn=1,m_sheetRepeat=0;
    std::vector<settings360::Visit> m_sheetHistory;
    struct {bool motion=false;} m_settings;
    struct {void play(const char*){}} m_audio;
    struct App {nxui::Input inputState;nxui::Input& input(){return inputState;}} application;
    App& app(){return application;}
    void takeFacts(bool){}
    int powerCalls=0,profileCalls=0;
    void openNxePage(int page);
    void openPower(){++powerCalls;}
    void openUsers(){++profileCalls;}
    void backNxeSettings();
    void activateNxeSetting(int delta=1,bool stepsOnly=false);
    bool touchNxeSettings(float,float);
    bool updateSheet(float);
    static MenuItem action(const char* label,std::function<void()> run){MenuItem i;i.label=label;i.run=std::move(run);return i;}
    static MenuItem info(const char* label,std::function<std::string()> value){MenuItem i;i.kind=MenuItem::Info;i.label=label;i.value=std::move(value);return i;}
    static MenuItem slider(const char* label,std::function<int()> get,std::function<void(int)> set,const char* detail){
        MenuItem i;i.kind=MenuItem::Slider;i.label=label;i.detail=detail;
        i.value=[get](){return std::to_string(get())+"%";};i.level=[get](){return get()/100.f;};
        i.change=[get,set](int d){set(std::clamp(get()+d*10,0,100));};return i;
    }
    void buildNxeSettings();
};
#include "../projects/cards/src/SettingsBindings.inc"
#include "../projects/cards/src/SettingsNavigation.inc"
}
int main(){
    using namespace cards;using namespace cards::settings360;
    CardsActivity app;int volume=60,saves=0,moves=0,themePicker=0,customPicker=0,themeAccent=2;bool startup=true,signIn=true;
    auto callback=CardsActivity::action("Preview startup",[](){});
    auto toggle=callback;toggle.label="Startup animation";toggle.kind=CardsActivity::MenuItem::Toggle;
    toggle.run={};toggle.level=[&](){return startup?1.f:0.f;};toggle.change=[&](int){startup=!startup;++saves;};
    auto themeColour=callback;themeColour.label="Theme colour";themeColour.kind=CardsActivity::MenuItem::Choice;
    themeColour.value=[&](){return std::to_string(themeAccent);};
    themeColour.change=[&](int d){themeAccent=(themeAccent+d+7)%7;++saves;};
    auto signInToggle=toggle;signInToggle.label="Sign-in at startup";
    signInToggle.level=[&](){return signIn?1.f:0.f;};signInToggle.change=[&](int){signIn=!signIn;++saves;};
    auto inert=[](const char* name){return CardsActivity::info(name,[](){return "real getter fixture";});};
    app.m_sheet={
        {"Home","home",{inert("Custom pages"),inert("Sort games")}},
        {"Look","gear",{inert("Clock"),inert("Animations"),themeColour,inert("Theme profile"),CardsActivity::action("Reset profile theme",[&](){themeAccent=2;++saves;}),toggle,signInToggle,callback,inert("Launch animation"),CardsActivity::action("Change Theme",[&](){++themePicker;}),CardsActivity::action("Background",[&](){++customPicker;})}},
        {"Lock screen","moon",{inert("Lock screen")}},
        {"Sound","music",{CardsActivity::slider("Music volume",[&](){return volume;},[&](int v){volume=v;++saves;},"Volume")}},
        {"Notifications","bell",{inert("Notification cards")}},
        {"Storage","copy",{inert("Console memory"),inert("microSD card"),CardsActivity::action("Move software between console and microSD",[&](){++moves;})}},
        {"Internet","wifi",{inert("Network"),inert("IP address"),CardsActivity::action("Connection test",[](){})}},
        {"Bluetooth","bluetooth",{CardsActivity::action("Headphones and speakers",[](){}),CardsActivity::action("Pair headphones or a speaker",[](){})}},
        {"Console","power",{CardsActivity::action("Connect to a computer",[](){}),inert("Language"),inert("Auto-brightness"),inert("Console's TV output")}},
        {"Pictures","tag",{inert("Card pictures")}},
        {"Updates","download",{CardsActivity::action("Homebrew updates",[](){})}},
        {"About","dots",{inert("System version"),inert("Atmosphere")}}
    };
#ifdef SWITCHU_MENU
    app.m_sheet[8].rows.push_back(inert("Set the time automatically"));
    app.m_sheet[8].rows.push_back(inert("Date"));
#endif
    app.buildNxeSettings();
    assert(app.m_sheet.size()==Count);
    for(int page=0;page<Count;++page) {
        assert(app.m_sheet[page].name==titles[page]);
        assert(!app.m_sheet[page].rows.empty());
    }
    for(const auto page:{System,Console}) {
        assert(app.m_sheet[page].rows.size()==8);
        for(int row=0;row<8;++row) {
            const auto target=page==System?systemLinks[row].target:consoleLinks[row].target;
            assert(app.m_sheet[page].rows[row].label==(page==System?systemLinks[row].label:consoleLinks[row].label));
            app.m_sheetSec=page;app.m_sheetRow=row;app.m_sheetHistory.clear();
            app.m_sheet[page].rows[row].run();assert(app.m_sheetSec==target);
            app.backNxeSettings();assert(app.m_sheetSec==page && app.m_sheetRow==row);
        }
    }
    // Follow every production link, including Preferences, Family and Initial
    // Setup, to ensure all settings pages are reachable from System Settings.
    std::vector<int> pending{System};std::array<bool,Count> visited{};
    while(!pending.empty()) {
        const int page=pending.back();pending.pop_back();if(visited[page]) continue;visited[page]=true;
        for(const auto& item:app.m_sheet[page].rows) if(item.run) {
            app.m_sheetSec=page;app.m_sheetRow=0;app.m_sheetHistory.clear();item.run();
            if(app.m_sheetSec!=page) pending.push_back(app.m_sheetSec);
        }
    }
    for(const bool seen:visited) assert(seen);
    auto find=[&](Page page,const char* label)->CardsActivity::MenuItem&{
        auto& rows=app.m_sheet[page].rows;
        const auto it=std::find_if(rows.begin(),rows.end(),[&](const auto& item){return item.label==label;});
        assert(it!=rows.end());return *it;
    };
    find(Look,"Theme colour").change(1);assert(themeAccent==3 && find(Look,"Theme colour").value()=="3");
    find(Look,"Reset profile theme").run();assert(themeAccent==2);
    assert(find(Look,"Theme profile").kind==CardsActivity::MenuItem::Info);
    auto& music=find(settings360::Audio,"Music volume");const int oldSaves=saves;
    const auto beforeTheme=themePicker,beforeCustom=customPicker;
    find(Look,"Change Theme").run();find(Look,"Background").run();
    assert(themePicker==beforeTheme+1 && customPicker==beforeCustom+1);
    music.change(1);assert(volume==70 && saves==oldSaves+1 && music.value()=="70%");
    volume=30;assert(music.value()=="30%");
    find(Startup,"Startup animation").change(1);
    assert(!startup && find(Look,"Startup animation").level()==0.f);
    const int beforeSignIn=saves;find(Startup,"Sign-in at startup").change(1);
    assert(!signIn && saves==beforeSignIn+1 && !startup);
    const int oldMoves=moves;find(Memory,"Move software between console and microSD").run();assert(moves==oldMoves+1);
    find(Display,"Brightness").change(1);assert(brightnessWritten>.59f && brightnessWritten<.61f);
    assert(find(settings360::Audio,"Pair headphones or a speaker").run);
    assert(app.m_sheet[Vision].rows.front().kind==CardsActivity::MenuItem::Info);
    assert(app.m_sheet[Vision].rows.front().detail.find("cannot operate")!=std::string::npos);
    // Exercise actual production controller handlers, not a parallel model.
    auto press=[&](nxui::Button button){app.application.inputState.press(button);assert(app.updateSheet(.016f));};
    app.m_sheetSec=System;app.m_sheetRow=0;app.m_sheetHistory.clear();app.m_sheetOpen=true;
    press(nxui::Button::A);assert(app.m_sheetSec==Console);
    press(nxui::Button::DDown);assert(app.m_sheetRow==1);
    press(nxui::Button::A);assert(app.m_sheetSec==settings360::Audio);
    volume=30;press(nxui::Button::DRight);assert(volume==40);
    press(nxui::Button::DLeft);assert(volume==30);
    press(nxui::Button::B);assert(app.m_sheetSec==Console && app.m_sheetRow==1);
    press(nxui::Button::B);assert(app.m_sheetSec==System && app.m_sheetRow==0);
    // Touch navigation enters a category immediately, but focusing a value
    // changes nothing. Only a second tap or an explicit +/- applies a change.
    app.m_sheetScroll=0;app.m_sheetRowsIn=1;
    app.touchNxeSettings(250,145);assert(app.m_sheetSec==Console);
    app.m_sheetRowsIn=1;app.touchNxeSettings(250,193);assert(app.m_sheetSec==settings360::Audio);
    app.m_sheetRow=1;app.m_sheetScroll=0;app.m_sheetRowsIn=1;
    const int beforeTouch=saves;app.touchNxeSettings(250,145);assert(volume==30 && saves==beforeTouch);
    app.touchNxeSettings(250,145);assert(volume==40 && saves==beforeTouch+1);
    app.touchNxeSettings(700,255);assert(volume==30);
    app.touchNxeSettings(1000,255);assert(volume==40);
    app.touchNxeSettings(220,650);assert(app.m_sheetSec==Console && app.m_sheetRow==1);
    app.m_sheetClosing=true;const int stoppedRow=app.m_sheetRow;
    press(nxui::Button::DDown);assert(app.m_sheetRow==stoppedRow);app.m_sheetClosing=false;
    press(nxui::Button::Plus);assert(!app.m_sheetOpen && app.m_sheetHistory.empty());
#ifndef SWITCHU_MENU
    assert(find(Clock,"Console date and time").value()=="HOME frontend required");
#else
    assert(find(Clock,"Date").value()=="real getter fixture");
#endif
    for(int target=1;target<Count;++target) {
        int page=System,selected=5;std::vector<Visit> history;
        assert(enter(target,page,selected,history));assert(page==target && selected==0);
        assert(back(page,selected,history));assert(page==System && selected==5);
        assert(!back(page,selected,history));
        assert(!enter(Count,page,selected,history));
    }
    for(int count:{1,8,10,11,27}) for(int selected=0;selected<count;++selected) {
        const float scroll=topRow(selected,count)*pitch;
        const auto chosen=row(selected,scroll);
        assert(chosen.y>=rowY && chosen.y+chosen.h<=listBottom);
        assert(hitRow(chosen.x+10,chosen.y+10,count,scroll)==selected);
        assert(hitRow(665,chosen.y+10,count,scroll)==-1);
        assert(hitRow(chosen.x+10,listBottom,count,scroll)==-1);
    }
    for(int page=0;page<Count;++page) {
        std::vector<Visit> history;seedHistory(page,history);int selected=0,where=page;
        int depth=0;while(back(where,selected,history)) assert(++depth<=2);
        assert(where==System);
    }
    std::cout<<"All 23 settings pages reachable; real callback ownership, shared saved values, app/HOME availability, history and scrolling touch bounds passed\n";
}
