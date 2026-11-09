// The centered NXE Guide, extended with a Discord Friends blade.
std::vector<std::string> CardsActivity::guideRows() {
    if (m_guideView==1) return {"My Games","My Apps","My Pins","Search","Friends","Messages"};
    if(m_guideTab==guide360::Friends) {
        std::vector<std::string> labels;
        for(const auto& row:m_guideDmRows) labels.push_back(row.label);
        return labels;
    }
    switch (m_guideTab) {
        case 0: return {"Game Marketplace","Video Marketplace"};
        case 1: return {"Achievements","Quick Launch","My Games","My Apps","My Pins"};
        case 2: return {"Xbox Dashboard","Quick Launch","Connect to Xbox LIVE",current()?"Play "+current()->name:"Game Library"};
        case guide360::Media: return {"Music Player","Picture Library","Video Library","Previous Track","Play / Pause","Next Track"};
        default: return {"Profile","Preferences","System Settings","Storage","Audio","Network","Turn Off Console"};
    }
}
void CardsActivity::openGuide() {
    m_dashGuide=true;m_guideClosing=false;m_guideIn=m_settings.motion?0.f:1.f;
    m_guideTab=2;m_guideView=0;m_guidePos=m_guideFrom=2.f;m_guideTabIn=1.f;m_dashGuideRow=0;m_guideRowAt=0.f;m_guideNote.clear();
    m_guideInbox={};m_guideDmRows.clear();m_guideDmRefresh=-1;
    m_guideChat.clear();m_guideMessages.clear();m_guideChatDrag=false;
    m_guideSels[guide360::Friends]=0;
    m_audio.play("show_modal");
}
void CardsActivity::closeGuide(bool animate) {
    if (animate && m_settings.motion) m_guideClosing=true;
    else { m_dashGuide=false;m_guideClosing=false;m_guideIn=0.f;m_guideChat.clear();m_guideMessages.clear();m_guideChatDrag=false; }
    m_audio.play("hide_modal");
}
void CardsActivity::guideTab(int delta) {
    m_guideChat.clear();m_guideMessages.clear();m_guideChatDrag=false;
    if (m_guideView==0) m_guideSels[m_guideTab]=m_dashGuideRow;
    m_guideFrom=m_guidePos;
    m_guideTab=guide360::wrap(m_guideTab+delta);m_guideView=0;
    m_dashGuideRow=m_guideSels[m_guideTab];m_guideRowAt=static_cast<float>(m_dashGuideRow);
    m_guideNote.clear();m_guideTabIn=m_settings.motion?0.f:1.f;
    if(m_guideTab==guide360::Friends) {
#ifdef SWITCHU_MENU
        m_discord.activate();m_discordStarted=true;
#endif
        refreshGuideDms(true);m_guideRowAt=float(m_dashGuideRow);
    }
    m_audio.play("tab_transition");
}
void CardsActivity::guideBack() {
    if(m_guideChat.active){m_guideChat.clear();m_guideMessages.clear();m_guideChatDrag=false;m_guideNote.clear();m_audio.play("hide_modal");return;}
    if (m_guideView) {
        m_guideView=0;m_dashGuideRow=m_guideSels[m_guideTab];
        m_guideRowAt=static_cast<float>(m_dashGuideRow);m_guideNote.clear();
        m_audio.play("hide_modal");
    } else closeGuide(true);
}
void CardsActivity::dashGuideActivate() {
    m_audio.play("activation");
    const int row=m_dashGuideRow;
    if(m_guideTab==guide360::Friends && !m_guideView) {activateGuideDm();return;}
    auto quick=[&]() { m_guideSels[m_guideTab]=row;m_guideView=1;m_dashGuideRow=0;m_guideRowAt=0; };
    if (m_guideView==1) {
        if(row==4 || row==5) {guideTab(guide360::Friends-m_guideTab);return;}
        closeGuide(false);
        switch(row) {
            case 0:dashTab(3);break;
            case 1:dashTab(6);break;
            case 2:openPagePicker();break;
            case 3:dashSearch();break;
        }
    } else if (m_guideTab==0) {
        closeGuide(false);
        if (row==0) { Game shop;shop.action="eshop";openNintendo(shop); }
        else dashTab(4);
    } else if (m_guideTab==1) {
        if (row==1) { quick();return; }
        if (row==0 && !current()) { m_guideNote="Select a game in your library first.";return; }
        closeGuide(false);
        if (row==0) openAchievements(*current());
        else if (row==2) dashTab(3);
        else if (row==3) dashTab(6);
        else openPagePicker();
    } else if (m_guideTab==2) {
        if (row==1) { quick();return; }
        if (row==2) { m_guideNote="Xbox LIVE is unavailable on Nintendo Switch.";return; }
        closeGuide(false);
        if (row==0) dashTab(1);
        else if (current()) play(); else dashTab(3);
    } else if (m_guideTab==guide360::Media) {
        if (row>=3) {
            if (row==3) m_audio.skip(-1);
            else if(row==4) m_audio.togglePause(); else m_audio.skip(1);
            return;
        }
        closeGuide(false);
        if (row==0) openMusic();else if(row==1) openGallery();else dashTab(4);
    } else {
        closeGuide(false);
        switch(row) {
            case 0:openUsers();break;
            case 1:dashSettings("Look");break;
            case 2:dashSettings("Console");break;
            case 3:dashSettings("Storage");break;
            case 4:dashSettings("Sound");break;
            case 5:dashSettings("Internet");break;
            case 6:openPower();break;
        }
    }
}
void CardsActivity::guideLabel(nxui::Renderer& r,const std::string& label,float cx,float cy,nxui::Color tint) {
    if (!m_guideTtf) m_guideTtf=TTF_OpenFont((std::string(kAssets)+"fonts/selawksb.ttf").c_str(),
        std::max(1,int(18*m_fGuide.pixelScale())));
    if (!m_guideTtf) return;
    auto& texture=m_guideLabels[label];
    if (!texture) {
        SDL_Surface* raw=TTF_RenderUTF8_Blended(m_guideTtf,label.c_str(),SDL_Color{255,255,255,255});
        if (!raw) return;
        SDL_Surface* rgba=SDL_ConvertSurfaceFormat(raw,SDL_PIXELFORMAT_RGBA32,0);SDL_FreeSurface(raw);
        if (!rgba) return;
        std::vector<std::uint8_t> rotated(size_t(rgba->w)*rgba->h*4);
        for(int y=0;y<rgba->h;++y) for(int x=0;x<rgba->w;++x)
            std::memcpy(rotated.data()+(size_t(x)*rgba->h+rgba->h-1-y)*4,
                static_cast<std::uint8_t*>(rgba->pixels)+size_t(y)*rgba->pitch+x*4,4);
        texture=std::make_unique<nxui::Texture>();
        nxui::GpuDevice::EssentialScope essential;
        if (!texture->loadFromPixels(app().gpu(),r,rotated.data(),rgba->h,rgba->w)) texture.reset();
        SDL_FreeSurface(rgba);
    }
    if (texture) {
        const float w=texture->width()/m_fGuide.pixelScale(),h=texture->height()/m_fGuide.pixelScale();
        r.drawTexture(texture.get(),{cx-w/2,cy-h/2,w,h},tint);
    }
}
void CardsActivity::drawGuide(nxui::Renderer& r) {
    if(m_guideTab==guide360::Friends && !m_guideView){refreshGuideDms();drawGuideChats(r);return;}
    const float a=nxe::ease(m_guideIn),shift=(1.f-a)*18.f;
    const auto white=nxui::Color(0.95f,0.97f,0.98f,a);
    const auto dark=nxui::Color(0.27f,0.31f,0.32f,a);
    r.drawRect({0,0,1280,720},{0,0,0,0.80f*a});
    auto body=guide360::blade(m_guideTab,m_guideTab);
    const float horizontal=(m_guidePos-m_guideTab)*38.f;body.x+=horizontal;
    r.drawRoundedRect({guide360::left,212+shift,guide360::width,292},{0.76f,0.82f,0.83f,a},5);
    // Draw the furthest blades first, then the active dark body.
    for(int distance=guide360::Count-1;distance>=1;--distance) for(int sign:{-1,1}) {
        const int i=m_guideTab+distance*sign;
        if(i<0 || i>=guide360::Count) continue;
        auto b=guide360::blade(i,m_guideTab);b.y+=shift;
        r.drawRoundedRect({b.x,b.y+3,b.w,b.h},{0,0,0,0.25f*a},5);
        r.drawRoundedRect({b.x,b.y,b.w,b.h},{201/255.f,215/255.f,218/255.f,a},5);
        r.drawGradientRect({b.x+2,b.y+4,6,b.h-8},{0.88f,0.94f,0.95f,0.6f*a},{0.68f,0.74f,0.76f,0.3f*a});
        guideLabel(r,i==2?m_guideName:std::string(guide360::names[i]),b.x+19,b.y+b.h/2,dark);
    }
    r.drawRoundedRect({body.x+3,body.y+shift+6,body.w,body.h},{0,0,0,0.45f*a},6);
    r.drawRoundedRect({body.x,body.y+shift,body.w,body.h},{44/255.f,60/255.f,73/255.f,a},6);
    r.drawGradientRect({body.x+2,body.y+shift+2,body.w-4,28},{0.30f,0.37f,0.42f,0.30f*a},{0.17f,0.23f,0.28f,0.f});
    r.drawRect({body.x+46,body.y+shift+4,2,body.h-8},{0.19f,0.26f,0.31f,0.5f*a});
    const auto& name=m_guideTab==2?m_guideName:std::string(guide360::names[m_guideTab]);
    guideLabel(r,name,body.x+24,body.y+shift+155,white);
    r.drawCircle({body.x+24,body.y+shift+24},13,{0.50f,0.59f,0.63f,a});
    r.drawCircle({body.x+24,body.y+shift+24},10,{0.19f,0.26f,0.31f,a});
    r.drawLine({body.x+15,body.y+shift+16},{body.x+23,body.y+shift+12},nxeColor({0.55f,0.77f,0.15f,a}),4);
    // Real profile picture, console battery and clock replace the reference account.
    if(m_avatar && m_avatar->valid()) r.drawTexture(m_avatar.get(),{body.x+249,146+shift,48,48},white);
    else icon(r,"friends",body.x+273,170+shift,40,white);
    r.drawRectOutline({body.x+10,173+shift,27,18},white,2);
    r.drawRect({body.x+37,178+shift,3,8},white);
    const float battery=std::clamp(std::atoi(m_battery.c_str()),0,100)/100.f;
    for(int i=0;i<4;++i) if(battery>i/4.f) r.drawRect({body.x+14+i*5.f,177+shift,3,10},white);
    icon(r,"pad",body.x+65,182+shift,28,white);
    textMid(r,m_fLabel,m_clock,body.x+body.w-measured(m_fLabel,m_clock).x-2,184+shift,white,1.f);
    auto rows=guideRows();
    m_dashGuideRow=std::clamp(m_dashGuideRow,0,std::max(0,int(rows.size())-1));
    if(m_guideView) textMid(r,m_fBody,"Quick Launch",body.x+61,185+shift,white,0.95f);
    if(m_guideTab==guide360::Friends && !m_guideView)
        textMid(r,m_fBody,"Discord Direct Messages",body.x+61,185+shift,white,0.95f);
    const float rowOpacity=a*(0.65f+0.35f*nxe::ease(m_guideTabIn));
    auto chosen=guide360::row(0,m_guideTab);chosen.x+=horizontal;chosen.y+=m_guideRowAt*43.f+shift;
    r.drawRoundedRect({chosen.x,chosen.y,chosen.w,chosen.h},nxeColor({90/255.f,155/255.f,3/255.f,rowOpacity}),3);
    r.drawGradientRect({chosen.x+1,chosen.y+1,chosen.w-2,19},nxeColor({140/255.f,193/255.f,53/255.f,rowOpacity}),nxeColor({0.48f,0.70f,0.13f,rowOpacity}));
    r.drawLine({chosen.x+3,chosen.y+1},{chosen.x+chosen.w-3,chosen.y+1},nxeColor({0.80f,0.90f,0.62f,0.45f*rowOpacity}),1);
    for(int i=0;i<int(rows.size());++i) {
        auto box=guide360::row(i,m_guideTab);box.x+=horizontal;box.y+=shift;
        const bool gameRow=m_guideTab==2 && !m_guideView && i==3 && current();
        const float room=box.w-(gameRow?62:16);
        textMid(r,m_fGuide,fit(m_fGuide,rows[i],room,1.f),box.x+8,box.y+21,white.withAlpha(rowOpacity),1.f);
        if(gameRow) {
            const auto art=m_thumbs.find(current()->id);
            if(art!=m_thumbs.end() && art->second.tex) r.drawTexture(art->second.tex.get(),{box.x+box.w-42,box.y+4,32,32},white);
        }
    }
    if(m_guideTab==guide360::Media && !m_audio.trackName().empty())
        textMid(r,m_fBadge,fit(m_fBadge,m_audio.trackName(),460,1.f),body.x+61,493+shift,white,1.f);
    if(!m_guideNote.empty()) textMid(r,m_fBadge,fit(m_fBadge,m_guideNote,460,1.f),body.x+61,(m_guideTab==guide360::Friends?575.f:493.f)+shift,white,1.f);
    if(m_guideTab==0 && m_guideNote.empty()) textMid(r,m_fBadge,m_dashGuideRow==0?"Opens Nintendo eShop":"Opens installed media apps",body.x+61,493+shift,white,1.f);
    if(m_guideTab==guide360::Friends && m_guideNote.empty()) {
        const auto note=m_guideDmState==homenx::dmview::State::Ready?
            "DMs "+std::to_string(m_guideInbox.conversations.size())+"  |  Page "+std::to_string(m_guideInbox.page+1)+"/"+std::to_string(m_guideInbox.pages())+"  |  * Unread":"X  Open Discord";
        textMid(r,m_fBadge,note,body.x+61,575+shift,white,0.9f);
    }
    const char* buttons[]={"A","B","X","Y"};
    const char* labels[]={"Select","Back",m_guideTab==guide360::Friends?"Discord":"Profile","Xbox Dashboard"};
    const float x[]={22,130,226,362};
    const nxui::Color colors[]={{0.44f,0.69f,0.08f,a},{0.68f,0.08f,0.08f,a},{0.08f,0.35f,0.69f,a},{0.83f,0.86f,0.08f,a}};
    for(int i=0;i<4;++i) {
        const float cx=body.x+x[i],cy=538+shift;
        r.drawCircle({cx,cy},9,colors[i]);
        textMid(r,m_fBadge,buttons[i],cx-4,cy,{1,1,1,a},0.8f);
        textMid(r,m_fPill,labels[i],cx+13,cy,white,0.95f);
    }
}
