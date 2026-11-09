// Software layout preview using production Guide chat geometry and drawing
// primitives. Sample identities/messages below are fictional; no account access.
#include "../projects/cards/src/GuideChat360.hpp"
#include <iostream>
#include <string>
using namespace cards::guidechat360;
struct Svg {
    int id=0;
    static std::string escaped(const std::string& s){std::string out;for(char c:s){if(c=='&')out+="&amp;";else if(c=='<')out+="&lt;";else if(c=='>')out+="&gt;";else out+=c;}return out;}
    static std::string color(Color c){return "rgb("+std::to_string(int(c.r*255))+","+std::to_string(int(c.g*255))+","+std::to_string(int(c.b*255))+")";}
    void rect(Box b,Color c){rounded(b,c,0);}
    void rounded(Box b,Color c,float radius){std::cout<<"<rect x='"<<b.x<<"' y='"<<b.y<<"' width='"<<b.w<<"' height='"<<b.h<<"' rx='"<<radius<<"' fill='"<<color(c)<<"' opacity='"<<c.a<<"'/>\n";}
    void gradient(Box b,Color t,Color d){const auto n=id++;std::cout<<"<defs><linearGradient id='g"<<n<<"' x1='0' y1='0' x2='0' y2='1'><stop stop-color='"<<color(t)<<"' stop-opacity='"<<t.a<<"'/><stop offset='1' stop-color='"<<color(d)<<"' stop-opacity='"<<d.a<<"'/></linearGradient></defs><rect x='"<<b.x<<"' y='"<<b.y<<"' width='"<<b.w<<"' height='"<<b.h<<"' fill='url(#g"<<n<<")'/>\n";}
    void circle(float x,float y,float r,Color c){std::cout<<"<circle cx='"<<x<<"' cy='"<<y<<"' r='"<<r<<"' fill='"<<color(c)<<"' opacity='"<<c.a<<"'/>\n";}
    void line(float x,float y,float xx,float yy,Color c,float width){std::cout<<"<path d='M"<<x<<","<<y<<" L"<<xx<<","<<yy<<"' stroke='"<<color(c)<<"' stroke-width='"<<width<<"'/>\n";}
    void text(const std::string& value,float x,float y,int size,Color c){std::cout<<"<text x='"<<x<<"' y='"<<y+size*.35f<<"' font-family='Selawik, sans-serif' font-size='"<<size<<"' fill='"<<color(c)<<"'>"<<escaped(value)<<"</text>\n";}
};
int main(int argc,char** argv){
    const bool chat=argc>1 && std::string(argv[1])=="chat";const int accent=argc>2?std::stoi(argv[2]):2;Svg p;
    const Color dark{.17f,.19f,.19f},white{.98f,.98f,.96f},muted{.35f,.38f,.37f};
    std::cout<<"<svg xmlns='http://www.w3.org/2000/svg' width='1280' height='720'>\n";
    p.gradient({0,0,1280,720},{.10f,.20f,.055f},{.03f,.05f,.025f});shell(p,1);chatIcon(p,224,117,false,dark);
    p.text(chat?"Weekend group":"Chat",250,118,27,dark);p.text("2:54 PM",966,75,23,white);chatIcon(p,1032,117,chat,muted);
    if(!chat){
        const char* names[]={"Alex","Sam","Weekend group","Jordan","Family chat"};
        for(int i=0;i<5;++i){auto b=row(i);if(i==2)selection(p,b,1,accent);else p.gradient(b,{.91f,.93f,.92f},{.81f,.84f,.83f});p.rect({b.x,b.y+b.h,b.w,1},{.61f,.65f,.64f});const auto color=i==2?white:dark;chatIcon(p,b.x+27,b.y+24,i==2 || i==4,color);p.text(names[i],b.x+55,b.y+22,24,color);p.text(i==2?"Unread - Group chat - 4 members":i==4?"Group chat - 3 members":"Direct message",b.x+55,b.y+43,17,color);}
        p.text("Weekend group",682,179,27,white);p.text("Discord group conversation",682,223,24,white);
        p.text("Open this chat to read",682,283,23,white);p.text("messages and reply from",682,311,23,white);p.text("the Guide.",682,339,23,white);p.text("Unread messages",682,399,23,white);p.text("Page 1 / 1",214,593,21,muted);
    }else{
        p.gradient({202,150,876,458},{.88f,.90f,.89f},{.75f,.79f,.77f});p.text("ZL / touch: load older messages",212,167,18,muted);p.text("4 members",850,167,18,muted);
        std::vector<Message> messages={{1,"Alex","Hey! Are you joining us later?","Today at 14:51"},{2,"You","Yes, I'll be there in a few minutes.","Today at 14:52",true},{3,"Sam","Great, see you soon!","Today at 14:53"},{4,"You","Just replying from the Guide.","Sending...",true,true}};
        Transcript transcript;transcript.replace(messages,[](const auto& text){return wrap(text,799,[](const auto& value){return value.size()*12.f;});});
        for(const auto& entry:transcript.entries){const auto y=history.y+entry.top-transcript.top();p.text(entry.message.author,history.x+9,y+14,22,entry.message.mine?cards::theme360::tint({.18f,.38f,.025f},accent):dark);p.text(entry.message.time,history.x+440,y+14,18,muted);for(std::size_t i=0;i<entry.lines.size();++i)p.text(entry.lines[i],history.x+9,y+42+i*27,23,dark);p.rect({history.x+6,y+entry.height-7,history.w-46,1},{.59f,.64f,.61f,.35f});}
        selection(p,compose,1,accent);p.text("A  Write a message...",compose.x+14,compose.y+22,25,white);
    }
    auto button=[&](float x,const char* letter,const char* label,Color color){p.circle(x,650,10,color);p.text(letter,x-5,650,15,white);p.text(label,x+17,650,21,white);};
    button(205,"A",chat?"Reply":"Select",{.39f,.65f,.035f});button(354,"B",chat?"Chats":"Back",{.74f,.1f,.08f});button(494,"X",chat?"Reply":"Discord",{.08f,.38f,.72f});button(680,"Y","Dashboard",{.82f,.71f,.06f});
    if(chat)p.text("L/R page  ZL older  ZR newest",823,650,17,white);
    std::cout<<"</svg>\n";
}
