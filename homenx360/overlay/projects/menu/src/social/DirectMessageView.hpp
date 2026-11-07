#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace homenx::dmview {
struct Conversation {
    std::uint64_t channelId=0;
    std::string name{},avatarUrl{},status{};
    bool unread=false;
    int mentions=0,members=0;
};
enum class State {HomeRequired,SignedOut,Connecting,Ready,Failed};
enum class Action {Conversation,Previous,Next,OpenDiscord,Info};
struct Row {Action action=Action::Info;std::string label;std::uint64_t channelId=0;};
template<class Channels> bool contains(const Channels& channels,std::uint64_t id) {
    return id && std::any_of(channels.begin(),channels.end(),[&](const auto& channel){return channel.channelId==id;});
}
template<class Channels> bool allowChat(bool directOnly,std::uint64_t channel,std::uint64_t guild,const Channels& dms) {
    return channel && (!directOnly || (!guild && contains(dms,channel)));
}
struct Draft {std::string account;std::uint64_t channel=0;bool directOnly=false;};
enum class Submit {Empty,AccountChanged,Unavailable,NotADm,Queued};
// The caller supplies the existing service's enqueue operation. This never
// sends during list refresh, navigation, or merely opening a conversation.
template<class Channels,class Enqueue> Submit submit(const Draft& draft,const std::string& current,
        bool ready,const Channels& dms,const std::string& text,Enqueue enqueue) {
    if(text.find_first_not_of(" \t\r\n\v\f")==std::string::npos) return Submit::Empty;
    if(draft.account.empty() || current!=draft.account) return Submit::AccountChanged;
    if(!ready || !draft.channel) return Submit::Unavailable;
    if(draft.directOnly && !contains(dms,draft.channel)) return Submit::NotADm;
    enqueue(draft.channel,text);return Submit::Queued;
}
struct Inbox {
    static constexpr std::size_t perPage=5;
    std::string account;
    std::vector<Conversation> conversations;
    std::size_t page=0;
    void update(std::string identity,std::vector<Conversation> incoming) {
        if(identity!=account){conversations.clear();page=0;account=std::move(identity);}
        // Keep visible conversations in place across presence/unread updates.
        // New conversations append until this Guide session is reopened.
        std::vector<Conversation> updated;
        for(const auto& old:conversations) {
            auto it=std::find_if(incoming.begin(),incoming.end(),[&](const auto& dm){return dm.channelId==old.channelId;});
            if(it!=incoming.end() && it->channelId){updated.push_back(std::move(*it));it->channelId=0;}
        }
        for(auto& dm:incoming) if(dm.channelId && !contains(updated,dm.channelId)) updated.push_back(std::move(dm));
        conversations=std::move(updated);
        page=std::min(page,pages()-1);
    }
    std::size_t pages()const{return std::max(std::size_t(1),(conversations.size()+perPage-1)/perPage);}
    void move(int delta){page=static_cast<std::size_t>(std::clamp(static_cast<long long>(page)+delta,0LL,static_cast<long long>(pages()-1)));}
    std::vector<Row> rows(State state)const {
        if(state==State::HomeRequired)return {{Action::Info,"Discord DMs require HOME"},{Action::OpenDiscord,"Open external Discord app"}};
        if(state==State::SignedOut)return {{Action::OpenDiscord,"Sign in to Discord"}};
        if(state==State::Connecting)return {{Action::Info,"Connecting to Discord..."},{Action::OpenDiscord,"Open Discord"}};
        if(state==State::Failed)return {{Action::Info,"Discord connection unavailable"},{Action::OpenDiscord,"Reconnect / sign in"}};
        if(conversations.empty())return {{Action::Info,"No direct messages yet"},{Action::OpenDiscord,"Open Discord"}};
        std::vector<Row> out;
        if(page)out.push_back({Action::Previous,"Previous conversations"});
        const auto end=std::min(conversations.size(),(page+1)*perPage);
        for(std::size_t i=page*perPage;i<end;++i) {
            const auto& dm=conversations[i];
            out.push_back({Action::Conversation,(dm.unread?"* ":"")+(dm.name.empty()?"Direct Message":dm.name),dm.channelId});
        }
        if(page+1<pages())out.push_back({Action::Next,"More conversations"});
        return out;
    }
};
} // namespace homenx::dmview
