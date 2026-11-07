#include "../projects/menu/src/social/DirectMessageView.hpp"
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <set>

// Exercise the production Guide methods with an in-memory Discord service.
class DiscordService {
public:
    enum class State {SignedOut,QrLogin,Connecting,Ready,Failed,TokenLogin};
    State current=State::Ready;std::string identity="account-a";
    std::vector<homenx::dmview::Conversation> channels;
    State state()const{return current;}
    const std::string& currentAccount()const{return identity;}
    auto dms(std::size_t)const{return channels;}
};
struct DiscordPanel {
    int opened=0;std::uint64_t channel=0,guild=0;
    bool isOpen()const{return true;}
    void openChat(std::uint64_t id,std::uint64_t group,const std::string&,const std::string&){++opened;channel=id;guild=group;}
};
namespace cards {
inline std::string plain(const std::string& s){return s;}
class CardsActivity {
public:
    float m_time=1,m_guideDmRefresh=-1,m_guideRowAt=0;
    int m_dashGuideRow=0,openedDiscord=0;bool directOnly=false,closed=false;
    std::string m_guideNote;
    homenx::dmview::Inbox m_guideInbox;
    std::vector<homenx::dmview::Row> m_guideDmRows;
    homenx::dmview::State m_guideDmState=homenx::dmview::State::HomeRequired;
    DiscordService m_discord;
    std::shared_ptr<DiscordPanel> m_discordPanel=std::make_shared<DiscordPanel>();
    struct {void play(const char*){}} m_audio;
    void closeGuide(bool){closed=true;}
    void openDiscord(bool only){++openedDiscord;directOnly=only;}
    void refreshGuideDms(bool force=false);
    void activateGuideDm();
};
#include "../projects/cards/src/GuideDiscord.inc"
}

int main() {
    using namespace homenx::dmview;
    Inbox inbox;
    for(const auto state:{State::HomeRequired,State::SignedOut,State::Connecting,State::Ready,State::Failed}) {
        const auto rows=inbox.rows(state);assert(!rows.empty() && rows.size()<=7);
        for(const auto& row:rows) assert(row.action!=Action::Conversation && row.channelId==0);
    }
    for(int count:{0,1,5,6,10,11,40,241}) {
        std::vector<Conversation> dms;
        for(int i=0;i<count;++i)dms.push_back({std::uint64_t(100+i),"Friend "+std::to_string(i),{},"online",i==0,0,i==2?3:0});
        inbox={};inbox.update("account-a",dms);std::set<std::uint64_t> seen;
        for(std::size_t page=0;page<inbox.pages();++page) {
            inbox.page=page;const auto rows=inbox.rows(State::Ready);assert(rows.size()<=7);
            for(const auto& row:rows) if(row.action==Action::Conversation) {
                assert(contains(dms,row.channelId));assert(seen.insert(row.channelId).second);
            }
            if(count) {
                assert((rows.front().action==Action::Previous)==(page>0));
                assert((rows.back().action==Action::Next)==(page+1<inbox.pages()));
            }
        }
        assert(seen.size()==std::size_t(count));
        inbox.move(10000);assert(inbox.page==inbox.pages()-1);inbox.move(-10000);assert(inbox.page==0);
        if(count) assert(inbox.rows(State::Ready).front().label.rfind("* ",0)==0);
    }
    inbox={};inbox.update("account-a",{{11,"Alice"},{22,"Bob"},{33,"Group DM"}});
    // Presence/unread events and new recent messages cannot swap the selected
    // person merely because Discord reordered its most-recent-DM list.
    inbox.update("account-a",{{22,"Bob",{},"idle",true},{44,"New DM"},{11,"Alice"},{33,"Group DM"}});
    assert(inbox.conversations[0].channelId==11 && inbox.conversations[1].channelId==22);
    assert(inbox.conversations[1].unread && inbox.conversations[1].status=="idle");
    assert(inbox.conversations.back().channelId==44);
    inbox.update("account-a",{{22,"Bob"},{22,"Duplicate"},{0,"Invalid"}});
    assert(inbox.conversations.size()==1 && inbox.conversations.front().channelId==22);
    inbox.update("account-b",{{99,"Other account"}});
    assert(inbox.account=="account-b" && inbox.conversations.size()==1 && inbox.conversations.front().channelId==99);
    assert(!contains(inbox.conversations,22));
    const std::vector<Conversation> dms={{11,"Alice"},{33,"Group"}};
    assert(allowChat(true,11,0,dms));assert(allowChat(true,33,0,dms));
    assert(!allowChat(true,11,555,dms));assert(!allowChat(true,999,0,dms));
    assert(!allowChat(true,0,0,dms));assert(allowChat(false,999,555,dms));
    int enqueued=0;std::uint64_t destination=0;std::string body;
    const auto send=[&](std::uint64_t channel,const std::string& text){++enqueued;destination=channel;body=text;};
    Draft draft{"account-a",11,true};
    for(const auto& empty:{""," ","\t\n\r"})assert(submit(draft,"account-a",true,dms,empty,send)==Submit::Empty);
    assert(submit(draft,"account-b",true,dms,"hello",send)==Submit::AccountChanged);
    assert(submit(draft,"account-a",false,dms,"hello",send)==Submit::Unavailable);
    assert(submit({"account-a",999,true},"account-a",true,dms,"hello",send)==Submit::NotADm);
    assert(submit({"account-a",0,true},"account-a",true,dms,"hello",send)==Submit::Unavailable);
    assert(enqueued==0);
    assert(submit(draft,"account-a",true,dms,"hello",send)==Submit::Queued);
    assert(enqueued==1 && destination==11 && body=="hello");
    assert(submit({"account-a",33,true},"account-a",true,dms,"group reply",send)==Submit::Queued);
    assert(enqueued==2 && destination==33);
    // The ordinary Discord entry keeps its existing server-chat capability.
    assert(submit({"account-a",999,false},"account-a",true,dms,"server reply",send)==Submit::Queued);
    assert(enqueued==3 && destination==999);
    cards::CardsActivity guide;
    guide.m_discord.channels={{11,"Alice"},{22,"Bob"},{33,"Group DM"}};
    guide.refreshGuideDms(true);
#ifdef SWITCHU_MENU
    assert(guide.m_guideDmRows.size()==3);
    // Discord can change its order between a rendered frame and activation.
    // The visible Alice row must still open Alice, with guild ID zero.
    guide.m_discord.channels={{22,"Bob"},{33,"Group DM"},{11,"Alice"}};
    guide.activateGuideDm();assert(guide.directOnly && guide.m_discordPanel->channel==11 && guide.m_discordPanel->guild==0);
    guide.m_discord.channels={{22,"Bob"},{33,"Group DM"}};
    const int wasOpened=guide.m_discordPanel->opened;
    guide.activateGuideDm();assert(guide.m_discordPanel->opened==wasOpened && !guide.m_guideNote.empty());
    guide.m_discord.identity="account-b";
    guide.activateGuideDm();assert(guide.m_discordPanel->opened==wasOpened);
    guide.m_discord.channels={{88,"New account DM"}};guide.refreshGuideDms(true);
    assert(guide.m_guideInbox.account=="account-b" && guide.m_guideDmRows[0].channelId==88);
    guide.m_dashGuideRow=0;guide.activateGuideDm();assert(guide.m_discordPanel->channel==88);
    guide.m_discord.channels.clear();
    for(int i=0;i<13;++i)guide.m_discord.channels.push_back({std::uint64_t(100+i),"DM"});
    guide.refreshGuideDms(true);guide.m_dashGuideRow=5;guide.activateGuideDm();
    assert(guide.m_guideInbox.page==1 && guide.m_guideDmRows.front().action==Action::Previous);
    guide.activateGuideDm();assert(guide.m_guideInbox.page==0);
    guide.m_discord.current=DiscordService::State::SignedOut;guide.refreshGuideDms(true);
    guide.m_dashGuideRow=0;const int beforeSignIn=guide.openedDiscord;guide.activateGuideDm();
    assert(guide.openedDiscord==beforeSignIn+1 && guide.directOnly && !guide.closed);
#else
    assert(guide.m_guideDmState==State::HomeRequired);
    guide.m_dashGuideRow=0;guide.activateGuideDm();assert(guide.openedDiscord==0 && !guide.m_guideNote.empty());
    guide.m_dashGuideRow=1;guide.activateGuideDm();assert(guide.openedDiscord==1 && guide.directOnly && guide.closed);
#endif
    std::cout<<"DM paging, stable IDs, unread/group DMs, account changes, server exclusion and explicit-submit routing passed; no network messages sent\n";
}
