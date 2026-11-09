#include "../projects/cards/src/Settings.hpp"
#include "../projects/cards/src/SettingsScene.hpp"
#include <cassert>
#include <iostream>
namespace cards {
void writeLater(const std::string& path,std::string data){std::filesystem::create_directories(std::filesystem::path(path).parent_path());std::ofstream(path)<<data;}
void flushWrites(){}
}
using namespace cards;
using theme360::Color;
struct Paint {
    std::vector<Color> colors;
    void rect(settings360::Rect,Color c){colors.push_back(c);}
    void rounded(settings360::Rect,Color c,float){colors.push_back(c);}
    void gradient(settings360::Rect,Color a,Color b){colors.push_back(a);colors.push_back(b);}
    void line(float,float,float,float,Color c,float){colors.push_back(c);}
    void circle(float,float,float,Color c){colors.push_back(c);}
};
static bool equal(Color a,Color b){return a.r==b.r && a.g==b.g && a.b==b.b && a.a==b.a;}
int main(){
    const Color shade{.3f,.7f,.02f,.4f};
    assert(equal(theme360::tint(shade,2),shade));
    auto blue=theme360::tint(shade,1),white=theme360::tint(shade,0);
    assert(blue.b>blue.g && blue.g>blue.r && blue.a==shade.a);
    assert(white.r==white.g && white.g==white.b && white.a==shade.a);
    for(int i=0;i<7;++i)for(int j=i+1;j<7;++j)assert(!equal(theme360::accent(i),theme360::accent(j)));
    assert(equal(theme360::accent(-1),theme360::accent(2)) && theme360::normalize(7)==2);
    for(int part=0;part<3;++part){
        auto draw=[part](Paint& p,int accent){if(part==0)settings360::background(p,.8f,accent);else if(part==1)settings360::selection(p,{0,0,40,20},.8f,accent);else settings360::orb(p,.8f,accent);};
        Paint green,blue,pink,white;draw(green,2);draw(blue,1);draw(pink,3);draw(white,0);
        assert(green.colors.size()==blue.colors.size());int changed=0;
        for(size_t i=0;i<green.colors.size();++i){assert(green.colors[i].a==blue.colors[i].a);if(!equal(green.colors[i],blue.colors[i])){++changed;assert(!equal(blue.colors[i],pink.colors[i]));assert(white.colors[i].r==white.colors[i].g && white.colors[i].g==white.colors[i].b);}}
        assert(changed>0);
    }
    const auto previous=std::filesystem::current_path();const auto tmp=std::filesystem::temp_directory_path()/"homenx360-theme-test";
    std::filesystem::remove_all(tmp);std::filesystem::create_directories(tmp);std::filesystem::current_path(tmp);
    const auto a=theme360::userId(1,2),b=theme360::userId(3,4),c=theme360::userId(5,6);
    assert(a=="00000000000000010000000000000002" && theme360::userId(0,0).empty());
    // Migration honours legacy White (index zero) rather than assuming green.
    std::filesystem::create_directories("sdmc:/config/CardsNX");
    std::ofstream(Settings::kPath)<<R"({"accent":0,"wallpaper":"nxe:night","musicVolume":41})";
    Settings settings;settings.load();assert(settings.accent==0 && settings.musicVolume==41);
    settings.activateTheme(a);assert(settings.accent==0 && settings.wallpaper=="nxe:night");
    settings.accent=1;settings.wallpaper="nxe:rings-blue";settings.profileAvatars[a].value[0]=3;settings.save();
    assert(!settings.activateTheme(a) && settings.accent==1); // same account keeps current edits
    settings.activateTheme(b);assert(settings.accent==0 && settings.wallpaper=="nxe:night");
    settings.accent=3;settings.wallpaper="sdmc:/my-picture.png";settings.save();
    settings.activateTheme(a);assert(settings.accent==1 && settings.wallpaper=="nxe:rings-blue");settings.save();
    Settings restarted;restarted.load();restarted.activateTheme(b);
    assert(restarted.accent==3 && restarted.wallpaper=="sdmc:/my-picture.png" && restarted.profileAvatars.at(a).value[0]==3);
    // A new profile after restart inherits the original shared theme, not A's last active theme.
    restarted.activateTheme(c);assert(restarted.accent==0 && restarted.wallpaper=="nxe:night");
    restarted.activateTheme("");assert(restarted.accent==0);restarted.accent=6;restarted.wallpaper.clear();restarted.save();
    Settings guest;guest.load();assert(guest.accent==6 && guest.wallpaper.empty());guest.activateTheme(a);assert(guest.accent==1);
    guest.activateTheme(b);assert(guest.accent==3);guest.save();
    auto j=nlohmann::json::parse(std::ifstream(Settings::kPath));
    j["profileThemes"][a]["accent"]="bad";j["profileThemes"][b]["wallpaper"]=23;j["profileThemes"]["bad id"]={{"accent",4}};
    j["profileThemes"][c]={{"accent",999},{"wallpaper","nxe:night"}};
    std::ofstream(Settings::kPath)<<j.dump();Settings damaged;damaged.load();
    assert(!damaged.profileThemes.contains(a) && !damaged.profileThemes.contains(b) && damaged.profileThemes.size()==1);
    damaged.activateTheme(c);assert(damaged.accent==2 && damaged.profileAvatars.at(a).value[0]==3 && damaged.musicVolume==41);
    for(int i=10;i<60;++i)damaged.profileThemes[theme360::userId(i,1)]={1,""};damaged.save();Settings capped;capped.load();assert(capped.profileThemes.size()==32 && capped.profileThemes.contains(c));
    std::filesystem::current_path(previous);std::filesystem::remove_all(tmp);
    std::cout<<"Profile theme migration, switching, restart, malformed saves and seven-colour rendering passed.\n";
}
