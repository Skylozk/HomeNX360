#include "../projects/cards/src/GuideChat360.hpp"
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
namespace cards {
inline std::string plain(const std::string& s){return s;}
struct DiscordService {
    enum class State {SignedOut,Connecting,Ready};State status=State::Ready;
    std::string account="account-a";std::vector<homenx::dmview::Conversation> channels={{11,"Alice"},{22,"Bob"},{33,"Weekend group",{},"",false,0,4}};
    int opens=0,older=0,sends=0;std::uint64_t lastChannel=0;std::string lastText;
    State state()const{return status;}const std::string& currentAccount()const{return account;}
    auto dms(std::size_t)const{return channels;}
    void openChannel(std::uint64_t channel){++opens;lastChannel=channel;}
    void olderMessages(std::uint64_t channel){++older;lastChannel=channel;}
    void send(std::uint64_t channel,const std::string& text){++sends;lastChannel=channel;lastText=text;}
};
class CardsActivity {
public:
    DiscordService m_discord;homenx::dmview::Inbox m_guideInbox;
    guidechat360::Session m_guideChat;guidechat360::Transcript m_guideMessages;
    std::uint64_t m_guideChatVersion=0;float m_guideChatRefresh=-1;bool m_guideChatDrag=false,m_guideChatDragged=false;
    bool m_guideChatLoading=false,m_guideChatOlder=false,m_guideChatError=false,m_guideChatStart=false,m_guideChatCapped=false;
    std::string m_guideNote;int keyboards=0;std::function<void(const std::string&)> accepted;
    void enterText(const std::string&,const std::string&,const std::string&,int max,std::function<void(const std::string&)> done,void*){assert(max==2000);++keyboards;accepted=std::move(done);}
    void openGuideConversation(std::uint64_t channel);void replyGuideConversation();void loadGuideHistory();
};
#include "../projects/cards/src/GuideChatActions.inc"
}
int main(){
    using namespace cards::guidechat360;
    for(int n=1;n<=7;++n)for(int i=0;i<n;++i){const auto box=row(i);assert(box.x>=frame.x && box.y>=frame.y && box.y+box.h<frame.y+frame.h);assert(hitRow(box.x+box.w/2,box.y+box.h/2,n)==i);assert(hitRow(box.x+box.w,box.y,n)==-1);}
    for(auto box:{history,compose,older,back,scrollUp,scrollDown})assert(box.x>=frame.x && box.y>=frame.y && box.x+box.w<=frame.x+frame.w && box.y+box.h<=frame.y+frame.h);
    auto measure=[](const std::string& s){float width=0;for(std::size_t at=0;at<s.size();at+=codepointSize(s,at))width+=10;return width;};
    for(const auto& text:{std::string(2000,'a'),std::string("hello\nworld"),std::string("long URL https://example.com/")+std::string(400,'x'),std::string("你好 世界 🙂 🙂🙂"),std::string("line 1\n\nline 3")}){
        const auto lines=wrap(text,90,measure);assert(!lines.empty());for(const auto& line:lines)assert(measure(line)<=90);
        auto compact=[](const std::string& s){std::string out;for(char c:s)if(c!=' ' && c!='\r' && c!='\n' && c!='\t')out+=c;return out;};
        std::string joined;for(const auto& line:lines)joined+=line;assert(compact(joined)==compact(text));
    }
    assert(wrap("one\n\nthree",200,measure).size()==3);
    assert(readableText("hi\xF3\xB0\x80\x80",true)=="hi[emoji]\n[Image / attachment - open Discord to view]");
    Transcript transcript;std::vector<Message> messages;
    for(int i=1;i<=15;++i)messages.push_back({std::uint64_t(i),"Author","Message "+std::to_string(i),"12:00"});
    auto wrapText=[&](const std::string& text){return wrap(text,300,measure);};
    transcript.replace(messages,wrapText);assert(transcript.offset==0 && transcript.maximum()>0);
    int rewrapped=0;transcript.replace(messages,[&](const auto& text){++rewrapped;return wrapText(text);});assert(rewrapped==0);
    transcript.scroll(230);const float oldTop=transcript.top();std::uint64_t anchor=0;float within=0;
    for(const auto& entry:transcript.entries)if(entry.top+entry.height>oldTop){anchor=entry.message.id;within=oldTop-entry.top;break;}
    messages.push_back({16,"Me","reply","12:01",true,true,false});transcript.replace(messages,wrapText);
    for(const auto& entry:transcript.entries)if(entry.message.id==anchor)assert(std::abs(transcript.top()-entry.top-within)<.01f);
    messages.insert(messages.begin(),{99,"Old","older history","11:00"});transcript.replace(messages,wrapText);
    for(const auto& entry:transcript.entries)if(entry.message.id==anchor)assert(std::abs(transcript.top()-entry.top-within)<.01f);
    transcript.scroll(-100000);assert(transcript.offset==0);messages.push_back({17,"Friend","new message","12:02"});transcript.replace(messages,wrapText);assert(transcript.offset==0);
    transcript.scroll(230);messages.push_back({18,"Friend","another new message","12:03"});transcript.replace(messages,wrapText);assert(transcript.newer && transcript.offset>0);transcript.scroll(-100000);assert(!transcript.newer);
    transcript.scroll(100000);assert(transcript.offset==transcript.maximum());transcript.clear();assert(transcript.entries.empty() && transcript.offset==0);
    transcript.replace({{88,"Author",std::string(8000,'x'),"12:00"}},wrapText);
    assert(transcript.entries.front().lines.size()>200);
    auto visible=transcript.visibleLines(transcript.entries.front());assert(visible.second-visible.first<20 && visible.second==transcript.entries.front().lines.size());
    transcript.scroll(100000);visible=transcript.visibleLines(transcript.entries.front());assert(visible.first==0 && visible.second<20);

    cards::CardsActivity app;app.m_guideInbox.update("account-a",app.m_discord.channels);
    app.openGuideConversation(999);assert(!app.m_guideChat.active);
#ifdef SWITCHU_MENU
    app.openGuideConversation(33);assert(app.m_guideChat.active && app.m_guideChat.members==4 && app.m_discord.opens==1 && app.m_discord.lastChannel==33);
    app.replyGuideConversation();assert(app.keyboards==1 && app.m_discord.sends==0); // opening/cancelling a keyboard sends nothing
    app.accepted(" \n\t");assert(app.m_discord.sends==0);
    app.accepted("group reply");assert(app.m_discord.sends==1 && app.m_discord.lastChannel==33 && app.m_discord.lastText=="group reply");
    app.replyGuideConversation();auto oldKeyboard=app.accepted;app.m_discord.account="account-b";oldKeyboard("wrong account");assert(app.m_discord.sends==1 && !app.m_guideChat.active && !app.m_guideNote.empty());
    app.m_discord.account="account-a";app.openGuideConversation(11);app.replyGuideConversation();oldKeyboard=app.accepted;
    app.openGuideConversation(22);oldKeyboard("wrong chat");assert(app.m_discord.sends==1 && app.m_guideChat.target.channel==22);
    app.replyGuideConversation();app.m_discord.channels.erase(app.m_discord.channels.begin()+1);app.accepted("removed DM");assert(app.m_discord.sends==1 && !app.m_guideChat.active);
    app.openGuideConversation(11);app.m_discord.status=cards::DiscordService::State::Connecting;const int count=app.keyboards;app.replyGuideConversation();assert(app.keyboards==count && !app.m_guideChat.active);
    app.m_discord.status=cards::DiscordService::State::Ready;app.openGuideConversation(11);app.loadGuideHistory();assert(app.m_discord.older==1);
    app.m_guideChatError=true;app.m_guideMessages.replace({{77,"You","queued reply","12:00",true,true}},wrapText);
    const auto opens=app.m_discord.opens;app.loadGuideHistory();assert(app.m_discord.opens==opens+1);
    const auto older=app.m_discord.older;app.m_discord.account="account-b";app.loadGuideHistory();assert(app.m_discord.older==older && app.m_discord.opens==opens+1);
#else
    app.openGuideConversation(11);app.replyGuideConversation();app.loadGuideHistory();assert(!app.m_guideChat.active && app.keyboards==0 && app.m_discord.opens==0 && app.m_discord.sends==0);
#endif
    std::cout<<"Guide chat geometry, Unicode/full message wrapping, anchored history, direct/group routing and explicit-reply account guards passed; no network messages sent.\n";
}
