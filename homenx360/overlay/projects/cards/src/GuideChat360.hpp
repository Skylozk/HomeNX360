#pragma once
#include "GuideLayout.hpp"
#include "Theme360.hpp"
#include "../../menu/src/social/DirectMessageView.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace cards::guidechat360 {
using Box=guide360::Box;
inline constexpr Box frame{192,96,896,528},list{202,150,456,458},detail{668,150,410,458};
inline constexpr Box history{212,188,856,342},compose{212,554,856,44},older{212,153,370,29};
inline constexpr Box back{192,96,62,44},scrollUp{1041,188,27,36},scrollDown{1041,494,27,36};
inline Box row(int i){return {list.x,list.y+i*58.f,list.w,56};}
inline int hitRow(float x,float y,int count){for(int i=0;i<std::min(7,count);++i)if(row(i).contains(x,y))return i;return -1;}
inline bool conversationScreen(int tab,int view){return tab==guide360::Friends && view==0;}
// User message text bypasses UI translation. Preserve Unicode/newlines and
// split long URLs at codepoint boundaries instead of silently truncating them.
inline std::size_t codepointSize(const std::string& s,std::size_t at){
    const auto c=static_cast<unsigned char>(s[at]);std::size_t n=c<0x80?1:c<0xE0?2:c<0xF0?3:4;
    if(at+n>s.size())return 1;for(std::size_t j=1;j<n;++j)if((static_cast<unsigned char>(s[at+j])&0xC0)!=0x80)return 1;return n;
}
template<class Measure> std::vector<std::string> wrap(const std::string& text,float width,Measure measure){
    std::vector<std::string> lines;std::string line;float lineWidth=0;
    for(std::size_t at=0;at<text.size();){
        const auto n=codepointSize(text,at);const auto next=text.substr(at,n);at+=n;
        if(next=="\r")continue;if(next=="\n"){lines.push_back(line);line.clear();lineWidth=0;continue;}
        const auto part=next=="\t"?std::string("    "):next;
        const float partWidth=measure(part);
        if(!line.empty() && lineWidth+partWidth>width){
            const auto space=line.find_last_of(' ');
            if(space!=std::string::npos && space>0){lines.push_back(line.substr(0,space));line=line.substr(space+1);}
            else {lines.push_back(line);line.clear();}
            lineWidth=line.empty()?0:measure(line);
            if(!line.empty() && lineWidth+partWidth>width){lines.push_back(line);line.clear();lineWidth=0;}
        }
        line+=part;lineWidth+=partWidth;
    }
    if(!line.empty() || lines.empty())lines.push_back(line);return lines;
}
inline std::string readableText(const std::string& text,bool media){
    std::string out;
    for(std::size_t at=0;at<text.size();){const auto n=codepointSize(text,at);const auto c=static_cast<unsigned char>(text[at]);
        // HomeNX uses private-use codepoints for downloaded emoji pictures.
        if(n==4 && c==0xF3 && static_cast<unsigned char>(text[at+1])>=0xB0)out+="[emoji]";
        else out+=text.substr(at,n);at+=n;
    }
    if(media)out+=(out.empty()?"":"\n")+std::string("[Image / attachment - open Discord to view]");
    return out.empty()?"[Message]":out;
}
struct Message {
    std::uint64_t id=0;std::string author,text,time;bool mine=false,pending=false,failed=false;
    bool operator==(const Message&)const=default;
};
struct Entry {Message message;std::vector<std::string> lines;float top=0,height=0;};
struct Transcript {
    std::vector<Entry> entries;float height=0,offset=0;bool newer=false;
    float maximum()const{return std::max(0.f,height-history.h);}
    float top()const{return maximum()-offset;}
    void scroll(float delta){offset=std::clamp(offset+delta,0.f,maximum());if(offset<.5f)newer=false;}
    void clear(){entries.clear();height=offset=0;newer=false;}
    std::pair<std::size_t,std::size_t> visibleLines(const Entry& entry)const{
        const auto count=static_cast<long long>(entry.lines.size());
        const auto first=std::clamp(static_cast<long long>(std::ceil((top()-entry.top-60)/27.f)),0LL,count);
        const auto last=std::clamp(static_cast<long long>(std::floor((top()+history.h-entry.top-24)/27.f))+1,first,count);
        return {static_cast<std::size_t>(first),static_cast<std::size_t>(last)};
    }

    template<class Wrap> void replace(std::vector<Message> messages,Wrap wrapText){
        if(messages.size()==entries.size() && std::equal(messages.begin(),messages.end(),entries.begin(),[](const auto& message,const auto& entry){return message==entry.message;}))return;
        std::uint64_t anchor=0;float within=0;const bool reading=offset>.5f;
        if(reading && !entries.empty() && !messages.empty() && messages.back().id>entries.back().message.id && !messages.back().mine)newer=true;
        if(reading)for(const auto& entry:entries)if(entry.top+entry.height>top()){anchor=entry.message.id;within=top()-entry.top;break;}
        entries.clear();height=0;
        for(auto& message:messages){Entry entry;entry.message=std::move(message);entry.lines=wrapText(entry.message.text);
            entry.top=height;entry.height=34.f+entry.lines.size()*27.f+16.f;height+=entry.height;entries.push_back(std::move(entry));}
        if(!reading)offset=0;
        else if(anchor)for(const auto& entry:entries)if(entry.message.id==anchor){offset=maximum()-entry.top-within;break;}
        offset=std::clamp(offset,0.f,maximum());
    }
};
struct Session {
    bool active=false;homenx::dmview::Draft target;std::string name;int members=0;
    void clear(){active=false;target={};name.clear();members=0;}
    void open(const std::string& account,const homenx::dmview::Conversation& dm){clear();if(account.empty() || !dm.channelId)return;
        active=true;target={account,dm.channelId,true};name=dm.name.empty()?"Direct message":dm.name;members=dm.members;}
    template<class Channels> bool valid(const std::string& account,bool ready,const Channels& dms)const{
        return active && ready && !target.account.empty() && account==target.account && homenx::dmview::contains(dms,target.channel);
    }
};
// Shared native/host preview drawing; actual conversation content comes from
// the caller. Controller button glyphs retain their physical colours.
using Color=theme360::Color;
template<class Paint> void shell(Paint& p,float a){
    p.rounded({frame.x+5,frame.y+8,frame.w,frame.h},{0,0,0,.45f*a},3);
    p.rounded(frame,{.95f,.96f,.94f,a},3);
    p.gradient({frame.x+2,frame.y+2,frame.w-4,42},{1,1,1,a},{.79f,.82f,.81f,a});
    p.gradient(list,{.89f,.91f,.90f,a},{.77f,.80f,.79f,a});
    p.gradient(detail,{.57f,.60f,.61f,a},{.36f,.39f,.40f,a});
    p.rect({frame.x+2,frame.y+43,frame.w-4,2},{.64f,.68f,.67f,a});
}
template<class Paint> void selection(Paint& p,Box box,float a,int accent){
    p.gradient(box,theme360::tint({.56f,.77f,.17f,a},accent),theme360::tint({.28f,.56f,.015f,a},accent));
    p.rect({box.x+1,box.y+1,box.w-2,1},{1,1,1,.42f*a});
}
template<class Paint> void chatIcon(Paint& p,float x,float y,bool group,Color color){
    if(group){p.circle(x-7,y-4,5,color);p.circle(x+7,y-4,5,color);p.rounded({x-17,y+3,18,13},color,4);p.rounded({x+1,y+3,17,13},color,4);}
    else {p.rounded({x-15,y-11,30,22},color,3);p.line(x-9,y+10,x-15,y+17,color,4);p.rect({x-9,y-5,18,2},{.86f,.88f,.87f,color.a});p.rect({x-9,y+1,14,2},{.86f,.88f,.87f,color.a});}
}
}
