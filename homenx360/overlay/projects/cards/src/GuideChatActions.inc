// Native Guide conversation actions. The same backend account/channel guard
// is checked when opening and again after explicit keyboard submission.
void CardsActivity::openGuideConversation(std::uint64_t channel){
#ifdef SWITCHU_MENU
    const auto dms=m_discord.dms(std::numeric_limits<size_t>::max());
    if(m_discord.state()!=DiscordService::State::Ready || m_discord.currentAccount()!=m_guideInbox.account)return;
    const auto found=std::find_if(dms.begin(),dms.end(),[&](const auto& dm){return dm.channelId==channel;});if(found==dms.end())return;
    m_guideChat.open(m_discord.currentAccount(),{found->channelId,plain(found->name),found->avatarUrl,found->status,found->unread,found->mentions,found->members});
    m_guideMessages.clear();m_guideChatVersion=~std::uint64_t(0);m_guideChatRefresh=-1;m_guideChatDrag=m_guideChatDragged=false;
    m_guideChatLoading=true;m_guideChatOlder=m_guideChatError=m_guideChatStart=m_guideChatCapped=false;
    m_guideNote.clear();m_discord.openChannel(channel);
#else
    (void)channel;
#endif
}
void CardsActivity::replyGuideConversation(){
#ifdef SWITCHU_MENU
    const auto dms=m_discord.dms(std::numeric_limits<size_t>::max());
    if(!m_guideChat.valid(m_discord.currentAccount(),m_discord.state()==DiscordService::State::Ready,dms)){
        m_guideChat.clear();m_guideMessages.clear();m_guideNote="Discord account or connection changed. Reopen the conversation.";return;
    }
    const auto draft=m_guideChat.target;
    enterText("Message "+m_guideChat.name,"Write a Discord message",std::string(),2000,[this,draft](const std::string& text){
        if(!m_guideChat.active || m_guideChat.target.account!=draft.account || m_guideChat.target.channel!=draft.channel){m_guideNote="Conversation changed. Open it again before replying.";return;}
        const auto result=homenx::dmview::submit(draft,m_discord.currentAccount(),m_discord.state()==DiscordService::State::Ready,
            m_discord.dms(std::numeric_limits<size_t>::max()),text,[this](std::uint64_t channel,const std::string& body){m_discord.send(channel,body);});
        if(result==homenx::dmview::Submit::Queued){m_guideMessages.offset=0;m_guideMessages.newer=false;m_guideChatVersion=~std::uint64_t(0);m_guideNote.clear();}
        else if(result!=homenx::dmview::Submit::Empty){m_guideChat.clear();m_guideMessages.clear();m_guideNote="Message was not sent. Discord account or connection changed.";}
    },nullptr);
#endif
}
void CardsActivity::loadGuideHistory(){
#ifdef SWITCHU_MENU
    const auto dms=m_discord.dms(std::numeric_limits<size_t>::max());
    if(!m_guideChat.valid(m_discord.currentAccount(),m_discord.state()==DiscordService::State::Ready,dms))return;
    // openChannel retries an initial load even when a queued/local reply is
    // already visible. For loaded chats it is a no-op; olderMessages itself
    // rejects initial/in-flight loads, so these cannot start parallel fetches.
    m_discord.openChannel(m_guideChat.target.channel);
    m_discord.olderMessages(m_guideChat.target.channel);
#endif
}
