// Host layout preview, using the native settings scene primitives. This is not
// a Switch framebuffer capture; native icons/font rasterization may differ.
#include "../projects/cards/src/SettingsScene.hpp"
#include <iostream>
#include <string>
#include <sstream>
using namespace cards::settings360;
struct Svg {
    int next=0;
    static std::string color(Color c){return "rgb("+std::to_string(int(c.r*255))+","+std::to_string(int(c.g*255))+","+std::to_string(int(c.b*255))+")";}
    static std::string escape(std::string s){
        std::string out;for(char c:s){if(c=='&')out+="&amp;";else if(c=='<')out+="&lt;";else if(c=='>')out+="&gt;";else out+=c;}return out;
    }
    void rect(Rect b,Color c){rounded(b,c,0);}
    void rounded(Rect b,Color c,float radius){std::cout<<"<rect x='"<<b.x<<"' y='"<<b.y<<"' width='"<<b.w<<"' height='"<<b.h<<"' rx='"<<radius<<"' fill='"<<color(c)<<"' opacity='"<<c.a<<"'/>\n";}
    void gradient(Rect b,Color a,Color d){
        int id=next++;std::cout<<"<defs><linearGradient id='g"<<id<<"' x1='0' y1='0' x2='0' y2='1'><stop stop-color='"<<color(a)<<"' stop-opacity='"<<a.a<<"'/><stop offset='1' stop-color='"<<color(d)<<"' stop-opacity='"<<d.a<<"'/></linearGradient></defs>\n";
        std::cout<<"<rect x='"<<b.x<<"' y='"<<b.y<<"' width='"<<b.w<<"' height='"<<b.h<<"' fill='url(#g"<<id<<")'/>\n";
    }
    void line(float x,float y,float xx,float yy,Color c,float width){std::cout<<"<path d='M"<<x<<","<<y<<" L"<<xx<<","<<yy<<"' stroke='"<<color(c)<<"' stroke-width='"<<width<<"' opacity='"<<c.a<<"'/>\n";}
    void circle(float x,float y,float radius,Color c){std::cout<<"<circle cx='"<<x<<"' cy='"<<y<<"' r='"<<radius<<"' fill='"<<color(c)<<"' opacity='"<<c.a<<"'/>\n";}
    void text(std::string s,float x,float y,int size,Color c){std::cout<<"<text x='"<<x<<"' y='"<<y<<"' font-family='Selawik' font-size='"<<size<<"' fill='"<<color(c)<<"'>"<<escape(s)<<"</text>\n";}
};
int main(int argc,char** argv){
    const bool console=argc>1 && std::string(argv[1])=="console";
    Svg p;std::cout<<"<svg xmlns='http://www.w3.org/2000/svg' width='1280' height='720' viewBox='0 0 1280 720'>\n";
    background(p,1);frame(p,1);orb(p,1);selection(p,row(0),1);
    p.text(console?"Console Settings":"System Settings",95,79,34,{.88f,.92f,.80f});
    const auto& links=console?consoleLinks:systemLinks;
    for(int i=0;i<8;++i) {
        const auto b=row(i);
        if(i>=1)p.rect({b.x+4,b.y+b.h-1,b.w-8,1.5f},{.65f,.72f,.75f,.25f});
        if(!console) {
            systemIcon(p,i,b.x+26,b.y+24,1);
        }
        p.text(links[i].label,b.x+(console?13:54),b.y+32,26,i==0?Color{.93f,.95f,.96f}:Color{.70f,.74f,.76f});
    }
    std::istringstream words(links[0].detail);std::string word,line;int y=156;
    while(words>>word){if(line.size()+word.size()>30){p.text(line,665,y,26,{.93f,.95f,.96f});line.clear();y+=30;}line+=(line.empty()?"":" ")+word;}
    if(!line.empty())p.text(line,665,y,26,{.93f,.95f,.96f});
    p.circle(111,650,11,{.39f,.63f,.08f});p.circle(221,650,11,{.65f,.12f,.13f});
    p.text("A",105,656,17,{.12f,.16f,.10f});p.text("B",215,656,17,{.12f,.16f,.10f});
    p.text("Select",131,657,21,{.97f,.97f,.94f});p.text("Back",241,657,21,{.97f,.97f,.94f});
    std::cout<<"</svg>\n";
}
