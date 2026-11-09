// Guide DM rows are refreshed during rendering, after this frame's input.
// Activation therefore always refers to the rows the player last saw.
void CardsActivity::refreshGuideDms(bool force) {
    if(!force && m_time<m_guideDmRefresh) return;
    m_guideDmRefresh=m_time+0.3f;
    const std::uint64_t keep=m_dashGuideRow>=0 && m_dashGuideRow<int(m_guideDmRows.size())?
        m_guideDmRows[m_dashGuideRow].channelId:0;
#ifdef SWITCHU_MENU
    using State=homenx::dmview::State;
    switch(m_discord.state()) {
        case DiscordService::State::Ready:m_guideDmState=State::Ready;break;
        case DiscordService::State::Connecting:m_guideDmState=State::Connecting;break;
        case DiscordService::State::Failed:m_guideDmState=State::Failed;break;
        default:m_guideDmState=State::SignedOut;break;
    }
    std::vector<homenx::dmview::Conversation> incoming;
    if(m_guideDmState==State::Ready) for(const auto& dm:m_discord.dms(std::numeric_limits<size_t>::max()))
        incoming.push_back({dm.channelId,plain(dm.name),dm.avatarUrl,dm.status,dm.unread,dm.mentions,dm.members});
    m_guideInbox.update(m_discord.currentAccount(),std::move(incoming));
#else
    m_guideDmState=homenx::dmview::State::HomeRequired;
#endif
    m_guideDmRows=m_guideInbox.rows(m_guideDmState);
    if(keep) {
        const auto it=std::find_if(m_guideDmRows.begin(),m_guideDmRows.end(),[&](const auto& row){return row.channelId==keep;});
        if(it!=m_guideDmRows.end()) m_dashGuideRow=int(it-m_guideDmRows.begin());
    }
    m_dashGuideRow=std::clamp(m_dashGuideRow,0,std::max(0,int(m_guideDmRows.size())-1));
}
void CardsActivity::activateGuideDm() {
    if(m_dashGuideRow<0 || m_dashGuideRow>=int(m_guideDmRows.size())) return;
    const auto row=m_guideDmRows[m_dashGuideRow];
    using Action=homenx::dmview::Action;
    if(row.action==Action::Info) {
        m_guideNote=m_guideDmState==homenx::dmview::State::HomeRequired?
            "Install the HOME frontend for built-in Discord DMs.":
            "X opens Discord sign-in, accounts and direct messages.";
        return;
    }
    if(row.action==Action::Previous || row.action==Action::Next) {
        m_guideInbox.move(row.action==Action::Next?1:-1);
        m_guideDmRows=m_guideInbox.rows(m_guideDmState);
        m_dashGuideRow=0;m_guideRowAt=0;m_audio.play("tab_transition");return;
    }
    if(row.action==Action::OpenDiscord) {
#ifndef SWITCHU_MENU
        closeGuide(false);
#endif
        openDiscord(true);return;
    }
#ifdef SWITCHU_MENU
    const auto dms=m_discord.dms(std::numeric_limits<size_t>::max());
    if(m_discord.state()!=DiscordService::State::Ready || m_discord.currentAccount()!=m_guideInbox.account
        || !homenx::dmview::contains(dms,row.channelId)) {
        m_guideNote="Conversation changed. Select it again after reconnecting.";
        refreshGuideDms(true);return;
    }
    const auto found=std::find_if(dms.begin(),dms.end(),[&](const auto& dm){return dm.channelId==row.channelId;});
    openGuideConversation(found->channelId);
#endif
}
