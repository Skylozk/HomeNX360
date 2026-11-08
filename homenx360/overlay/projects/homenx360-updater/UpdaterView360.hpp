#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace homenx360::updaterui {
constexpr int width=1280,height=720;
constexpr uint32_t rgb(int r,int g,int b){return uint32_t(r)|(uint32_t(g)<<8)|(uint32_t(b)<<16)|0xff000000u;}
inline uint32_t mix(uint32_t a,uint32_t b,int alpha){uint32_t result=0xff000000;for(int shift:{0,8,16})result|=uint32_t((((a>>shift)&255)*(255-alpha)+((b>>shift)&255)*alpha)/255)<<shift;return result;}
inline std::vector<uint8_t> read(const std::string& path,size_t limit){std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f || f.tellg()<0 || uint64_t(f.tellg())>limit)return {};std::vector<uint8_t> b(size_t(f.tellg()));f.seekg(0);if(!f.read(reinterpret_cast<char*>(b.data()),b.size()))return {};return b;}
struct Font {
    uint32_t cw=0,ch=0,w=0,h=0;std::array<uint16_t,96> advance{};std::vector<uint8_t> alpha;
    bool load(const std::string& path){auto b=read(path,2*1024*1024);if(b.size()<216 || std::memcmp(b.data(),"NXUFNT1\0",8))return false;
        std::memcpy(&cw,b.data()+8,4);std::memcpy(&ch,b.data()+12,4);std::memcpy(&w,b.data()+16,4);std::memcpy(&h,b.data()+20,4);
        if(!cw || cw>80 || !ch || ch>100 || w!=cw*16 || h!=ch*6 || b.size()!=216+size_t(w)*h)return false;
        std::memcpy(advance.data(),b.data()+24,192);for(auto a:advance)if(a>cw)return false;alpha.assign(b.begin()+216,b.end());return true;
    }
    int measure(const std::string& text)const{int result=0;for(unsigned char c:text)result+=advance[(c>=32 && c<128?c:'?')-32];return result;}
};
struct Canvas {
    uint32_t* pixels;int stride;
    void rect(int x,int y,int w,int h,uint32_t color){if(w<=0 || h<=0)return;for(int yy=std::max(0,y);yy<std::min(height,y+h);++yy)std::fill(pixels+yy*stride+std::clamp(x,0,width),pixels+yy*stride+std::clamp(x+w,0,width),color);}
    void gradient(int x,int y,int w,int h,uint32_t top,uint32_t bottom){for(int row=0;row<h;++row)rect(x,y+row,w,1,mix(top,bottom,row*255/std::max(1,h-1)));}
    void circle(int x,int y,int radius,uint32_t color){for(int yy=-radius;yy<=radius;++yy){const int half=int(std::sqrt(float(radius*radius-yy*yy)));rect(x-half,y+yy,half*2+1,1,color);}}
    void text(const Font& font,const std::string& text,int x,int y,uint32_t color){
        for(unsigned char c:text){const int glyph=(c>=32 && c<128?c:'?')-32;
            for(uint32_t yy=0;yy<font.ch;++yy){const int dy=y+int(yy);if(dy<0 || dy>=height)continue;
                for(uint32_t xx=0;xx<font.cw;++xx){const int dx=x+int(xx);if(dx<0 || dx>=width)continue;const uint8_t a=font.alpha[(glyph/16*font.ch+yy)*font.w+glyph%16*font.cw+xx];if(a)pixels[dy*stride+dx]=mix(pixels[dy*stride+dx],color,a);}}
            x+=font.advance[glyph];
        }
    }
    void wrapped(const Font& font,const std::string& text,int x,int y,int maxWidth,int maxLines,uint32_t color){
        size_t at=0;for(int line=0;line<maxLines && at<text.size();++line){std::string next;size_t lastSpace=std::string::npos;
            while(at<text.size() && text[at]!='\n'){const char c=text[at];if(font.measure(next+c)>maxWidth)break;next+=c;++at;if(c==' ')lastSpace=next.size()-1;}
            if(at<text.size() && text[at]!='\n' && lastSpace!=std::string::npos){at-=next.size()-lastSpace-1;next.resize(lastSpace);}
            if(next.empty() && at<text.size()){next=text.substr(at++,1);}
            if(at<text.size() && text[at]=='\n')++at;
            if(line+1==maxLines && at<text.size()){while(next.size()>3 && font.measure(next+"...")>maxWidth)next.pop_back();next+="...";}
            this->text(font,next,x,y+line*(font.ch-3),color);
        }
    }
};
enum class Phase {Checking,Downloading,Verifying,Preparing,Installing,Complete,Current,Failed,Canceled};
struct State {
    Phase phase=Phase::Checking;int step=0;bool running=true,applet=false,canceling=false;
    std::string installed,available,error;
    float download=0,install=0;uint64_t downloadBytes=0;double megabytesPerSecond=0;
};
class View {
    Font small,body,title;std::vector<uint32_t> background;std::vector<uint8_t> logo;
    static std::string percent(float n){return std::to_string(int(std::clamp(n,0.f,1.f)*100+.5f))+"%";}
public:
    bool load(const std::string& root){
        if(!small.load(root+"font-small.bin") || !body.load(root+"font-body.bin") || !title.load(root+"font-title.bin"))return false;
        logo=read(root+"logo.rgba",256*256*4);if(logo.size()!=256*256*4)return false;
        background.resize(width*height);Canvas c{background.data(),width};
        c.gradient(0,0,width,490,rgb(36,70,7),rgb(152,179,102));c.gradient(0,490,width,230,rgb(152,179,102),rgb(229,234,220));
        for(int i=0;i<24;++i)c.rect(i*57,0,1,170,rgb(66,98,30));
        c.rect(88,204,1104,393,rgb(55,72,44));c.gradient(85,199,1104,392,rgb(245,248,240),rgb(211,220,202));
        c.gradient(103,220,224,348,rgb(58,78,42),rgb(89,111,65));c.rect(348,220,1,348,rgb(175,188,159));
        for(int y=0;y<96;++y)for(int x=0;x<96;++x){const auto* p=&logo[(y*256/96*256+x*256/96)*4];background[(50+y)*width+81+x]=rgb(p[0],p[1],p[2]);}
        return true;
    }
    void draw(uint32_t* pixels,int stride,const State& state,float seconds)const{
        for(int y=0;y<height;++y)std::memcpy(pixels+y*stride,background.data()+y*width,width*4);
        Canvas c{pixels,stride};const auto dark=rgb(35,49,27),muted=rgb(86,103,70),white=rgb(250,252,245),green=rgb(103,173,14),red=rgb(165,38,29);
        c.text(title,"HomeNX360",198,47,white);c.text(body,"System Update",201,103,rgb(220,235,202));
        c.text(small,state.applet?"APPLET MODE":"APPLICATION MODE",1000,64,white);
        c.text(small,"Preview app + HOME dashboard",922,108,rgb(220,235,202));
        const char* steps[]={"Check release","Download","Verify files","Prepare files","Install both"};
        const int current=state.phase==Phase::Complete || state.phase==Phase::Current?5:std::clamp(state.step,0,4);
        for(int step=0;step<5;++step){const int y=247+step*59;const bool active=step==current,done=step<current;
            if(active)c.gradient(110,y-2,210,47,rgb(137,188,43),rgb(96,145,12));
            c.circle(132,y+21,10,done?green:active && state.phase==Phase::Failed?red:rgb(184,199,166));
            c.text(small,std::to_string(step+1),124,y+4,dark);c.text(small,steps[step],151,y+3,white);
        }
        const char* headings[]={"Checking for updates","Downloading update","Checking the download","Preparing the update","Installing both apps","Update complete","You're up to date","Update stopped","Update paused"};
        const int phase=int(state.phase);c.text(title,headings[phase],374,227,state.phase==Phase::Failed?red:dark);
        const std::string installed=state.installed=="unknown"?"Version not recorded":state.installed;
        c.text(small,"Installed: "+installed,376,288,muted);
        if(!state.available.empty())c.text(small,"Available: "+state.available,376,321,muted);
        if(state.phase==Phase::Failed){
            c.rect(373,365,765,124,rgb(244,222,211));c.wrapped(small,state.error,386,371,735,4,red);
            c.text(small,"Full details are saved in the updater log.",376,501,muted);
        }else{
            const float amount=state.phase==Phase::Downloading?state.download:state.phase==Phase::Complete || state.phase==Phase::Current?1.f:state.install;
            const bool indefinite=state.phase==Phase::Checking || state.phase==Phase::Verifying;
            c.rect(375,392,764,26,rgb(166,184,147));c.rect(379,396,756,18,rgb(218,229,205));
            if(indefinite){const int x=int((std::sin(seconds*2.f)+1)*.5f*596);c.gradient(379+x,396,160,18,rgb(149,208,49),green);}
            else {const int progress=int(756*std::clamp(amount,0.f,1.f));if(progress)c.gradient(379,396,progress,18,rgb(172,226,63),green);}
            std::string progress;
            if(state.phase==Phase::Downloading){progress=percent(state.download);
                if(state.downloadBytes){const auto mb=state.downloadBytes/(1024*1024);progress+="   "+std::to_string(uint64_t(mb*state.download))+" / "+std::to_string(mb)+" MB";}
                if(state.megabytesPerSecond>.01){char speed[48];std::snprintf(speed,sizeof(speed),"   %.1f MB/s",state.megabytesPerSecond);progress+=speed;}
            }else if(!indefinite)progress=percent(amount);
            c.text(body,progress,376,431,dark);
            std::string detail=state.canceling?"Pausing safely. Your partial download can resume.":state.phase==Phase::Checking?"Looking for the newest complete HomeNX360 release.":state.phase==Phase::Downloading?"Downloads resume if the connection is interrupted.":state.phase==Phase::Verifying?"Confirming the release checksum before installation.":state.phase==Phase::Preparing?"Extracting and checking the update on your SD card.":state.phase==Phase::Installing?"Keep the console powered on while files are replaced.":state.phase==Phase::Complete?"Restart the console to load the updated HOME dashboard.":state.phase==Phase::Current?"Both the preview app and HOME dashboard are current.":"Launch the updater again to continue the download.";
            c.wrapped(small,detail,376,487,760,2,muted);
        }
        c.text(small,"Your saved avatars and settings are preserved.",375,549,muted);
        auto button=[&](int x,const char* letter,const std::string& label,uint32_t color){c.circle(x,649,13,color);c.text(small,letter,x-7,632,white);c.text(body,label,x+24,625,dark);};
        if(!state.running)button(106,"A",state.phase==Phase::Failed?"Retry update":state.phase==Phase::Canceled?"Resume update":"Check again",green);
        if(state.phase!=Phase::Installing)button(state.running?106:405,"B",state.running?"Pause / Exit":"Exit",red);
        else c.text(body,"Installing - please wait",88,625,dark);
        c.text(small,"HomeNX360 Updater",918,637,muted);
        c.text(small,"config/SwitchU/homenx360-updater.log",375,683,muted);
    }
};
}
