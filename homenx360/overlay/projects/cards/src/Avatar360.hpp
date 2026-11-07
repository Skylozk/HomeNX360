#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cards::avatar360 {
enum Feature {Character,Hair,HairColour,Top,Bottom,Shoes,Accessories,Height,Build,FeatureCount};
inline constexpr std::array<const char*,FeatureCount> featureNames={
    "Face & body","Hairstyle","Hair colour","Top","Bottoms","Shoes","Accessories","Height","Build"
};
inline constexpr std::array<int,FeatureCount> choices={6,4,6,3,3,3,2,5,5};
inline constexpr std::array<std::array<float,3>,6> hairTints={{{1,1,1},{.055f,.047f,.04f},{.29f,.15f,.065f},{.90f,.74f,.32f},{.67f,.24f,.07f},{.78f,.79f,.80f}}};
inline constexpr std::array<const char*,6> presets={"The regular","The player","The fan","The street","The smart one","The night owl"};
inline constexpr std::array<const char*,6> modelFiles={"regular","player","fan","street","smart","night"};
// Each starter uses its original body family. Wardrobe parts only mix within
// that family; choosing a face/body resets the wardrobe to its matching set.
inline int donor(int character,int variation){return (character%2)+2*((character/2+variation)%3);}
struct Look {
    std::array<int,FeatureCount> value={0,0,0,0,0,0,1,2,2};
    void normalize(){for(int i=0;i<FeatureCount;++i)value[i]=std::clamp(value[i],0,choices[i]-1);}
    void cycle(int field,int delta){if(field>=0 && field<FeatureCount){const auto n=choices[field];value[field]=((value[field]+delta)%n+n)%n;if(field==Character)for(int k=Hair;k<=Shoes;++k)value[k]=0;}}
    bool operator==(const Look&)const=default;
};
inline int partSource(const Look& look,int slot){
    const auto& v=look.value;const int main=v[Character];
    switch(slot){
        case 2:return v[Hair]==3?-1:donor(main,v[Hair]);
        case 3:return donor(main,v[Top]);
        case 4:return donor(main,v[Bottom]);
        case 5:return donor(main,v[Shoes]);
        case 6:return v[Accessories]?main:-1;
        case 7:return v[Hair]!=3?donor(main,v[Hair]):-1; // hat-cut hair keeps its matching hat
        default:return main;
    }
}
inline bool partEnabled(const Look& look,int slot,int source){
    if(slot<8)return partSource(look,slot)==source;
    if(source!=look.value[Character])return false;
    if(slot==17)return source%2==0 && donor(source,look.value[Bottom])==2;
    if(slot>16)return false;
    const int region=(slot-8)/3,feature=region==0?Top:region==1?Bottom:Shoes;
    return (slot-8)%3==donor(source,look.value[feature])/2;
}
inline std::uint32_t hash(const std::string& id){std::uint32_t h=2166136261u;for(unsigned char c:id){h^=c;h*=16777619u;}return h;}
inline Look randomLook(std::uint32_t seed){
    Look look;auto next=[&](){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;};
    if(!seed)seed=0x360360u;
    for(int i=0;i<FeatureCount;++i)look.value[i]=int(next()%choices[i]);
    return look;
}
inline bool validId(const std::string& id){
    return id.size()==32 && id.find_first_not_of("0123456789ABCDEF")==std::string::npos
        && id.find_first_not_of('0')!=std::string::npos;
}
using Wardrobe=std::map<std::string,Look>;
inline Look forProfile(const Wardrobe& wardrobe,const std::string& id){
    const auto it=wardrobe.find(id);Look defaults;defaults.value[Character]=hash(id)%6;auto look=it==wardrobe.end()?defaults:it->second;look.normalize();return look;
}
inline std::string featureValue(const Look& look,int field){
    if(field<0 || field>=FeatureCount)return {};
    const int v=std::clamp(look.value[field],0,choices[field]-1);
    switch(field){
        case Character:return presets[v];
        case Hair:{static const char* names[]={"Short and spiky","Ponytail","High and tight + cap","A-line bob + cap","Left parting","Bob with fringe"};return v==3?"No hair":names[donor(look.value[Character],v)];}
        case HairColour:{static const char* names[]={"Original","Black","Brown","Blonde","Ginger","Silver"};return names[v];}
        case Top:{static const char* names[]={"Graphic tee","Halo tee","Basketball jersey","Trefoil tee","Blue button shirt","Band tee"};return names[donor(look.value[Character],v)];}
        case Bottom:{static const char* names[]={"Khaki chinos","Skinny jeans","Cargo shorts","Cuffed jeans","Khaki chinos","Dark flares"};return names[donor(look.value[Character],v)];}
        case Shoes:{static const char* names[]={"Casual sneakers","Neon high-tops","Lakers sneakers","Sports trainers","Casual sneakers","Goth boots"};return names[donor(look.value[Character],v)];}
        case Accessories:return v?"On":"Off";
        default:return std::to_string(v+1)+" / 5";
    }
}

struct Profile {std::string id,name;};
enum class Screen {Closed,Loading,Picker,Editor,Welcome};
struct Session {
    Screen screen=Screen::Closed;
    std::vector<Profile> profiles;
    int selected=0,field=0;
    Look draft;
    std::string editingId;
    float welcomeTime=0,yaw=0;
    bool atStartup=false;
    bool open()const{return screen!=Screen::Closed;}
    const Profile* current()const{return selected>=0 && selected<int(profiles.size())?&profiles[selected]:nullptr;}
    void begin(bool startup){screen=Screen::Loading;atStartup=startup;selected=0;field=0;profiles.clear();editingId.clear();}
    void loaded(std::vector<Profile> available,const std::string& active){
        if(screen!=Screen::Loading)return;
        profiles.clear();for(auto& profile:available)if(validId(profile.id) &&
            std::none_of(profiles.begin(),profiles.end(),[&](const auto& old){return old.id==profile.id;}))profiles.push_back(std::move(profile));
        selected=0;for(int i=0;i<int(profiles.size());++i)if(profiles[i].id==active)selected=i;
        screen=Screen::Picker;
    }
    void move(int delta){if(screen==Screen::Picker)selected=std::clamp(selected+delta,0,std::max(0,int(profiles.size())-1));}
    void edit(const Wardrobe& wardrobe){
        const auto* profile=current();if(screen!=Screen::Picker || !profile)return;
        editingId=profile->id;draft=forProfile(wardrobe,editingId);field=0;yaw=0;screen=Screen::Editor;
    }
    bool save(Wardrobe& wardrobe){
        if(screen!=Screen::Editor || !validId(editingId))return false;
        draft.normalize();wardrobe[editingId]=draft;editingId.clear();screen=Screen::Picker;return true;
    }
    void back(){
        if(screen==Screen::Editor){editingId.clear();screen=Screen::Picker;}
        else screen=Screen::Closed;
    }
    std::optional<Profile> signIn(){
        const auto* profile=current();if(screen!=Screen::Picker || !profile)return std::nullopt;
        return *profile;
    }
    void signedIn(){if(screen==Screen::Picker && current()){screen=Screen::Welcome;welcomeTime=0;}}
    void tick(float dt){if(screen==Screen::Welcome && (welcomeTime+=std::max(0.f,dt))>=5.5f)screen=Screen::Closed;}
};
inline int firstVisible(int selected,int count){return std::clamp(selected-1,0,std::max(0,count-4));}
struct Box {float x,y,w,h;bool contains(float px,float py)const{return px>=x && px<x+w && py>=y && py<y+h;}};
inline Box profileBox(int index,float scroll){return {85.f+(index-scroll)*335.f,140,210,480};}
inline int editorTop(int selected){return std::clamp(selected-7,0,std::max(0,FeatureCount-8));}
inline Box editorRow(int index,int top){return {650,140.f+(index-top)*44.f,520,42};}
} // namespace cards::avatar360
