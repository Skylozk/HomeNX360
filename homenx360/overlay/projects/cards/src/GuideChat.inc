// NXE Chat screen: silver private-conversation list, grey details, and an
// in-Guide message reader. No messages are sent from rendering or refresh.
void CardsActivity::refreshGuideConversation(){
    if(!m_guideChat.active)return;
#ifdef SWITCHU_MENU
    if(!m_guideChat.valid(m_discord.currentAccount(),m_discord.state()==DiscordService::State::Ready,m_guideInbox.conversations)){
        m_guideChat.clear();m_guideMessages.clear();m_guideChatDrag=false;
        m_guideNote="Discord account, connection or conversation changed. Select a chat again.";return;
    }
    const auto state=m_discord.history(m_guideChat.target.channel);
    m_guideChatLoading=state.loading;m_guideChatOlder=state.older;m_guideChatError=state.error;m_guideChatStart=state.start;m_guideChatCapped=state.capped;
    if(m_guideMessages.offset<.5f && !m_guideChatLoading)m_discord.markChannelRead(m_guideChat.target.channel);
    if(m_guideChatVersion==m_discord.version() || m_time<m_guideChatRefresh)return;
    m_guideChatRefresh=m_time+.3f;m_guideChatVersion=m_discord.version();
    std::vector<guidechat360::Message> messages;
    for(const auto& line:m_discord.lines(m_guideChat.target.channel,200,true))messages.push_back({line.id,plain(line.author),guidechat360::readableText(line.text,!line.media.empty()),line.time,line.mine,line.pending,line.failed});
    m_guideMessages.replace(std::move(messages),[this](const std::string& text){return guidechat360::wrap(text,guidechat360::history.w-57,[this](const std::string& value){return m_fGuide.measure(value).x*.9f;});});
#endif
}
void CardsActivity::drawGuideChats(nxui::Renderer& r){
    using namespace guidechat360;refreshGuideConversation();
    const float a=nxe::ease(m_guideIn),shift=(1-a)*18;
    struct Paint {
        nxui::Renderer& r;float shift;
        static nxui::Color color(Color c){return {c.r,c.g,c.b,c.a};}
        nxui::Rect box(Box b)const{return {b.x,b.y+shift,b.w,b.h};}
        void rect(Box b,Color c){r.drawRect(box(b),color(c));}
        void rounded(Box b,Color c,float radius){r.drawRoundedRect(box(b),color(c),radius);}
        void gradient(Box b,Color t,Color d){r.drawGradientRect(box(b),color(t),color(d));}
        void circle(float x,float y,float radius,Color c){r.drawCircle({x,y+shift},radius,color(c));}
        void line(float x,float y,float xx,float yy,Color c,float w){r.drawLine({x,y+shift},{xx,yy+shift},color(c),w);}
    } paint{r,shift};
    const nxui::Color dark{.17f,.19f,.19f,a},white{.98f,.98f,.96f,a},muted{.35f,.38f,.37f,a};
    auto raw=[&](nxui::Font& font,const std::string& text,float x,float y,nxui::Color color,float scale=1.f){r.drawText(text,{x,y+shift-font.measure("A").y*scale*.5f},&font,color,scale);};
    auto clipped=[&](nxui::Font& font,const std::string& text,float x,float y,float width,nxui::Color color,float scale=1.f){
        r.pushClipRect({x,y+shift-22,width,44});raw(font,text,x,y,color,scale);r.popClipRect();
    };
    r.drawRect({0,0,1280,720},{0,0,0,.72f*a});shell(paint,a);
    chatIcon(paint,224,117,false,{.19f,.22f,.21f,a});
    clipped(m_fGuide,m_guideChat.active?m_guideChat.name:"Chat",250,118,680,dark,1.05f);
    raw(m_fBody,m_clock,1018-measured(m_fBody,m_clock).x,75,white,.95f);
    chatIcon(paint,1032,117,m_guideChat.active && m_guideChat.members>0,{.42f,.45f,.44f,a});
    if(!m_guideChat.active){
        for(int i=0;i<int(m_guideDmRows.size());++i){
            const auto box=guidechat360::row(i);const auto& item=m_guideDmRows[i];const bool chosen=i==m_dashGuideRow;
            const homenx::dmview::Conversation* dm=nullptr;
            for(const auto& value:m_guideInbox.conversations)if(value.channelId==item.channelId){dm=&value;break;}
            if(chosen)selection(paint,box,a,m_settings.accent);
            else paint.gradient(box,{.91f,.93f,.92f,a},{.81f,.84f,.83f,a});
            paint.rect({box.x,box.y+box.h,box.w,1},{.61f,.65f,.64f,a});
            const auto color=chosen?white:dark;
            if(item.action==homenx::dmview::Action::Conversation)chatIcon(paint,box.x+27,box.y+24,dm && dm->members>0,{color.r,color.g,color.b,a});
            else raw(m_fGuide,item.action==homenx::dmview::Action::Next?">":item.action==homenx::dmview::Action::Previous?"<":"...",box.x+14,box.y+27,color);
            std::string label=dm?(dm->name.empty()?"Direct message":dm->name):item.label;
            clipped(m_fGuide,label,box.x+55,box.y+22,box.w-76,color,.92f);
            if(dm){const auto status=(dm->unread?std::string("Unread - "):std::string())+(dm->members>0?"Group chat - "+std::to_string(dm->members)+" members":dm->status.empty()?"Direct message":dm->status);
                clipped(m_fBody,status,box.x+55,box.y+43,box.w-77,color,.66f);
                if(dm->unread)paint.circle(box.x+box.w-13,box.y+20,4,{1,1,1,a});}
        }
        float y=179;
        auto paragraph=[&](const std::string& text,nxui::Color color,float scale=.88f){for(const auto& line:guidechat360::wrap(text,detail.w-28,[&](const auto& value){return m_fGuide.measure(value).x*scale;})){if(y>572)break;raw(m_fGuide,line,detail.x+14,y,color,scale);y+=28;}y+=16;};
        const homenx::dmview::Conversation* selected=nullptr;
        if(m_dashGuideRow>=0 && m_dashGuideRow<int(m_guideDmRows.size()))for(const auto& dm:m_guideInbox.conversations)if(dm.channelId==m_guideDmRows[m_dashGuideRow].channelId){selected=&dm;break;}
        if(selected){paragraph(selected->name.empty()?"Direct message":selected->name,white,1);paragraph(selected->members>0?"Discord group conversation":"Discord direct message",white);
            paragraph("Open this chat to read messages and reply from the Guide.",white);
            if(selected->unread)paragraph("Unread messages",white);
        }else{
            paragraph("Discord messages",white,1);
            switch(m_guideDmState){
                case homenx::dmview::State::HomeRequired:paragraph("Install the HOME dashboard for built-in Discord DMs and group chats. The preview NRO can open an external Discord app.",white);break;
                case homenx::dmview::State::SignedOut:paragraph("Sign in using HomeNX's existing Discord account screen. Your private conversations will appear here.",white);break;
                case homenx::dmview::State::Connecting:paragraph("Connecting to your Discord account...",white);break;
                case homenx::dmview::State::Failed:paragraph("Discord is unavailable. Open Discord to reconnect or sign in again.",white);break;
                default:paragraph("Your existing DMs and group chats appear here. Open Discord to start a new conversation.",white);break;
            }
        }
        if(!m_guideNote.empty())paragraph(m_guideNote,white,.75f);
        raw(m_fBody,"Page "+std::to_string(m_guideInbox.page+1)+" / "+std::to_string(m_guideInbox.pages()),list.x+12,593,muted,.8f);
    }else{
        paint.gradient({202,150,876,458},{.88f,.90f,.89f,a},{.75f,.79f,.77f,a});
        const auto topLabel=m_guideChatLoading?"Loading messages...":m_guideChatOlder?"Loading older messages...":m_guideChatError?"Couldn't load history - select to retry":m_guideChatCapped?"Latest 200 messages loaded":m_guideChatStart?"Beginning of conversation":"ZL / touch: load older messages";
        clipped(m_fBody,topLabel,older.x,older.y+14,530,muted,.72f);
        clipped(m_fBody,m_guideChat.members>0?std::to_string(m_guideChat.members)+" members":"Discord DM",850,167,206,muted,.72f);
        r.pushClipRect({history.x,history.y+shift,history.w-31,history.h});
        for(const auto& entry:m_guideMessages.entries){
            const float y=history.y+entry.top-m_guideMessages.top();if(y+entry.height<history.y || y>history.y+history.h)continue;
            const auto heading=entry.message.mine?nxeColor({.18f,.38f,.025f,a}):dark;
            clipped(m_fGuide,entry.message.mine?"You":entry.message.author,history.x+9,y+14,415,heading,.84f);
            const std::string state=entry.message.failed?"Failed to send":entry.message.pending?"Sending...":entry.message.time;
            clipped(m_fBody,state,history.x+440,y+14,350,entry.message.failed?nxui::Color(.7f,.1f,.08f,a):muted,.7f);
            const auto visible=m_guideMessages.visibleLines(entry);
            for(std::size_t line=visible.first;line<visible.second;++line)raw(m_fGuide,entry.lines[line],history.x+9,y+42+line*27,dark,.9f);
            paint.rect({history.x+6,y+entry.height-7,history.w-46,1},{.59f,.64f,.61f,.35f*a});
        }
        if(m_guideMessages.entries.empty() && !m_guideChatLoading && !m_guideChatError)raw(m_fGuide,"No messages yet. Press A to write one.",history.x+12,history.y+35,dark,.9f);
        r.popClipRect();
        if(m_guideMessages.maximum()>0){
            paint.rect({1052,229,4,252},{.56f,.61f,.58f,.5f*a});const float height=std::max(18.f,252*history.h/m_guideMessages.height);
            paint.rect({1052,229+(252-height)*(1-m_guideMessages.offset/m_guideMessages.maximum()),4,height},{.30f,.39f,.31f,a});
            raw(m_fGuide,"^",1045,205,dark,.9f);raw(m_fGuide,"v",1045,511,dark,.9f);
        }
        selection(paint,compose,a,m_settings.accent);
        raw(m_fGuide,"A  Write a message...",compose.x+14,compose.y+22,white,.94f);
        if(m_guideMessages.newer)clipped(m_fBody,"New messages below - ZR: newest",214,539,850,muted,.65f);
        else if(!m_guideNote.empty())clipped(m_fBody,m_guideNote,214,539,850,muted,.65f);
    }
    auto button=[&](float x,const char* letter,const char* label,nxui::Color color){r.drawCircle({x,650+shift},10,color);raw(m_fBadge,letter,x-4,650,white,.8f);raw(m_fBody,label,x+17,650,white,.85f);};
    button(205,"A",m_guideChat.active?"Reply":"Select",{.39f,.65f,.035f,a});button(354,"B",m_guideChat.active?"Chats":"Back",{.74f,.1f,.08f,a});
    button(494,"X",m_guideChat.active?"Reply":"Discord",{.08f,.38f,.72f,a});button(680,"Y","Dashboard",{.82f,.71f,.06f,a});
    if(m_guideChat.active)raw(m_fBody,"L/R page  ZL older  ZR newest",823,650,white,.65f);
}
bool CardsActivity::touchGuideChats(){
    using namespace guidechat360;auto& input=app().input();if(m_guideClosing)return true;
    const float x=input.touchX(),y=input.touchY()-(1-nxe::ease(m_guideIn))*18;
    if(m_guideChat.active && input.touchDown() && history.contains(x,y)){m_guideChatDrag=true;m_guideChatDragged=false;m_guideChatDragY=y;m_guideChatDragOffset=m_guideMessages.offset;}
    if(m_guideChatDrag && input.isTouching()){
        const float delta=y-m_guideChatDragY;if(std::abs(delta)>10)m_guideChatDragged=true;
        if(m_guideChatDragged)m_guideMessages.offset=std::clamp(m_guideChatDragOffset+delta,0.f,m_guideMessages.maximum());
        m_guideMessages.scroll(0);
    }
    if(!input.touchUp())return true;
    const bool dragged=m_guideChatDrag && m_guideChatDragged;m_guideChatDrag=false;if(dragged)return true;
    if(m_guideChat.active){
        if(compose.contains(x,y)){replyGuideConversation();return true;}
        if(older.contains(x,y)){loadGuideHistory();return true;}
        if(scrollUp.contains(x,y)){m_guideMessages.scroll(110);return true;}
        if(scrollDown.contains(x,y)){m_guideMessages.scroll(-110);return true;}
    }else {const int at=guidechat360::hitRow(x,y,m_guideDmRows.size());if(at>=0){m_dashGuideRow=at;activateGuideDm();return true;}}
    if(back.contains(x,y)){guideBack();return true;}
    if(y>=631 && y<673){
        if(x>=190 && x<335){if(m_guideChat.active)replyGuideConversation();else activateGuideDm();}
        else if(x<480 && x>=335)guideBack();
        else if(x>=480 && x<665){if(m_guideChat.active)replyGuideConversation();else openDiscord(true);}
        else if(x>=665 && x<817){closeGuide(false);dashTab(1);}
        return true;
    }
    if(!frame.contains(x,y))closeGuide(true);return true;
}
