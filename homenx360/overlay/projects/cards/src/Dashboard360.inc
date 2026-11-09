// Included inside namespace cards, after CardsActivity's drawing helpers.
bool CardsActivity::dashLibrary() const {
    return m_dashTab == 3 || m_dashTab == 4 || m_dashTab == 6;
}

int CardsActivity::dashCount() const {
    if (m_dashTab == 1) return 8;
    if (dashLibrary()) {
        return static_cast<int>(m_pages[m_page].size());
    }
    if (m_dashTab == 0) return 2;
    if (m_dashTab == 2) return 4;
    if (m_dashTab == 5) return 3;
    return 10;
}

void CardsActivity::dashBeginTab(int tab) {
    const int next = dash360::wrapTab(tab);
    if (next == m_dashTab) return;
    m_dashPrevTab = m_dashTab;
    m_dashPrevTile = m_dashTile;
    m_dashPrevCard = m_dashCard;
    m_dashPrevPage = m_page;
    // Preserve shoulder-button direction when wrapping between the end tabs.
    m_dashDir = tab < 0 ? -1 : tab >= 8 ? 1 : next < m_dashTab ? -1 : 1;
    m_dashTab = next;
    m_dashIn = m_settings.motion ? 0.f : 1.f;
}

float CardsActivity::dashSlide() const {
    return m_settings.motion ? m_dashDir * 490.f * (1.f - nxe::ease(m_dashIn)) : 0.f;
}

void CardsActivity::dashTab(int tab) {
    dashBeginTab(tab);
    m_dashTile = 0;
    m_zone = Zone::Cards;
    const int target = m_dashTab == 6 ? kHomebrew : m_dashTab == 4 ? kMedia : kGames;
    // Dashboard tabs own page selection, including a page hidden in CardsNX.
    if (m_page != target) {
        m_page = target;
        sel() = std::clamp(sel(), 0, std::max(0, static_cast<int>(games().size()) - 1));
        m_scroll = static_cast<float>(sel());
        m_pageIn = 1.f;
        requeueArt();
        wantArt();
    }
    if (dashLibrary()) m_dashTile = sel();
    m_dashCard = static_cast<float>(m_dashTile);
    m_audio.play("navigation");
}

void CardsActivity::dashSettings(const std::string& section) {
    using namespace settings360;
    const std::map<std::string,int> pages={{"Console",System},{"Look",Look},{"Sound",settings360::Audio},{"Internet",Network},
        {"Bluetooth",Remote},{"Storage",Memory},{"Home",Home},{"Notifications",Notifications},{"About",SystemInfo},
        {"Updates",Updates},{"Lock screen",Lock},{"Pictures",Pictures}};
    const auto found=pages.find(section);openSettings(found==pages.end()?System:found->second);
}

void CardsActivity::dashSearch() {
    askText("Search your library", "Find an installed game or homebrew app", "", 64, [this](const std::string& query) {
        if (query.empty()) return;
        auto lower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
            return s;
        };
        const auto q = lower(query);
        std::vector<MenuItem> rows;
        for (int page : {static_cast<int>(kGames), static_cast<int>(kHomebrew), static_cast<int>(kMedia)})
            for (const auto& g : m_pages[page])
                if (lower(g.name).find(q) != std::string::npos)
                    rows.push_back(action(g.name, [this, g, page]() {
                        dashTab(page == kHomebrew ? 6 : page == kMedia ? 4 : 3);
                        selectById(g.id);
                        m_dashTile = sel();
                    }));
        if (rows.empty()) { say("No matching games or apps."); return; }
        openMenu("Search results", std::move(rows));
    });
}

void CardsActivity::dashActivate() {
    m_audio.play("activation");
    if (dashLibrary()) { if (current()) play(); return; }
    if (m_dashTab == 0) {
        if (m_dashTile == 0) dashSearch(); else openBrowser();
    } else if (m_dashTab == 1) {
        switch (m_dashTile) {
            case 0: if (current()) play(); else dashTab(3); break;
            case 1: openUsers(); break;
            case 2: dashTab(3); break;
            case 3: dashTab(4); break;
            case 4: openMusic(); break;
            case 5: openGallery(); break;
            case 6: dashTab(6); break;
            case 7: openSettings(); break;
        }
    } else if (m_dashTab == 2) {
        switch (m_dashTile) {
            case 0:
#ifdef SWITCHU_MENU
                openSocial();
#else
                { std::string error; openSystem("friends", m_user, error); if (!error.empty()) say(error); }
#endif
                break;
            case 1: openUsers(); break;
            case 2: openDiscord(); break;
            case 3: openNotifications(); break;
        }
    } else if (m_dashTab == 5) {
        if (m_dashTile == 0) openMusic();
        else if (m_dashTile == 1) dashSettings("Sound");
        else dashTab(4);
    } else {
        static const char* sections[] = {"Console", "Look", "Sound", "Internet", "Bluetooth", "Storage", "Home", "Notifications", "About"};
        if (m_dashTile == 9) openPower();
        else dashSettings(sections[m_dashTile]);
    }
    // Page-picker actions can change the CardsNX page behind this shell.
}

void CardsActivity::dashMove(int delta) {
    const int next = nxe::nextCard(m_dashTile,delta,dashCount());
    if (next == m_dashTile) return;
    m_dashTile = next;
    if (dashLibrary()) select(next);
    if (!m_settings.motion) m_dashCard = static_cast<float>(next);
    m_audio.play("navigation");
}

bool CardsActivity::dashInput(float dt) {
    if (m_moving) return false;
    auto& in = app().input();
    using B = nxui::Button;
    const B directions[][3] = {{B::DLeft,B::LStickL,B::RStickL}, {B::DRight,B::LStickR,B::RStickR},
                              {B::DUp,B::LStickU,B::RStickU}, {B::DDown,B::LStickD,B::RStickD}};
    bool held=false, down=false;
    int direction=-1;
    for (int i=0;i<4;++i) for (auto button:directions[i]) {
        if (in.isDown(button)) { down=true; direction=i; }
        if (in.isHeld(button)) { held=true; if (direction<0) direction=i; }
    }
    if (down) m_dashRepeat=0.30f;
    else if (held && (m_dashRepeat-=dt)<=0.f) { down=true; m_dashRepeat=0.13f; }
    if (m_dashGuide) {
        if (m_guideClosing) return true;
        if (in.isDown(B::B)) { guideBack();return true; }
        if (in.isDown(B::Y)) { closeGuide(false);dashTab(1);return true; }
        if(m_guideChat.active){
            if(in.isDown(B::Plus)){closeGuide(true);return true;}
            if(in.isDown(B::A) || in.isDown(B::X)){replyGuideConversation();return true;}
            if(in.isDown(B::ZL)){loadGuideHistory();return true;}
            if(in.isDown(B::ZR)){m_guideMessages.scroll(-m_guideMessages.maximum());return true;}
            if(in.isDown(B::L))m_guideMessages.scroll(guidechat360::history.h*.85f);
            if(in.isDown(B::R))m_guideMessages.scroll(-guidechat360::history.h*.85f);
            if(down && direction>=2)m_guideMessages.scroll(direction==2?60.f:-60.f);
            return true;
        }
        if (in.isDown(B::X)) {
            if(m_guideTab==guide360::Friends) openDiscord(true);
            else {closeGuide(false);openUsers();}
            return true;
        }
        if (in.isDown(B::Plus)) { closeGuide(true);return true; }
        if (in.isDown(B::L)) guideTab(-1);
        if (in.isDown(B::R)) guideTab(1);
        if (down) {
            if (direction<2) guideTab(direction==0?-1:1);
            else {
                m_dashGuideRow=nxe::nextCard(m_dashGuideRow,direction==2?-1:1,guideRows().size());
                m_guideNote.clear();m_audio.play("navigation");
            }
        }
        if (in.isDown(B::A)) dashGuideActivate();
        return true;
    }
    if (in.isDown(B::Y)) { openGuide();return true; }
    if (in.isDown(B::ZL)) dashTab(m_dashTab-1);
    if (in.isDown(B::ZR)) dashTab(m_dashTab+1);
    if (in.isDown(B::L)) dashMove(-1);
    if (in.isDown(B::R)) dashMove(1);
    if (down) {
        if (direction<2) dashMove(direction==0?-1:1);
        else dashTab(m_dashTab+(direction==2?-1:1));
    }
    if (in.isDown(B::Minus)) openNotifications();
    if (in.isDown(B::X)) askClose();
    if (in.isDown(B::Plus) && current() && (dashLibrary() || (m_dashTab==1 && m_dashTile==0))) { openOptions(); return true; }
    if (in.isDown(B::B)) { dashTab(1); return true; }
    if (in.isDown(B::A)) dashActivate();
    return true;
}

bool CardsActivity::dashTouch() {
    if (m_pickOpen || m_spQrOpen || m_sheetOpen || m_notifOpen || m_quickOpen || m_powerOpen || m_dotsOpen
        || m_musicOpen || m_optOpen || !m_menu.empty() || m_moving) return handleTouch();
    auto& in=app().input();
    if(m_dashGuide && guidechat360::conversationScreen(m_guideTab,m_guideView))return touchGuideChats();
    if (!in.touchUp()) return in.isTouching();
    const float x=in.touchX(), y=in.touchY();
    if (m_dashGuide) {
        if (m_guideClosing) return true;
        const float gy=y-(1.f-nxe::ease(m_guideIn))*18.f;
        auto body=guide360::blade(m_guideTab,m_guideTab);
        const float horizontal=(m_guidePos-m_guideTab)*38.f;body.x+=horizontal;
        const int tab=body.contains(x,gy)?-1:guide360::hitBlade(x,gy,m_guideTab);
        if (tab>=0) { guideTab(tab-m_guideTab);return true; }
        if (gy>=146 && gy<194 && x>=body.x+249 && x<body.x+297) {
            closeGuide(false);openUsers();return true;
        }
        const int row=guide360::hitRow(x-horizontal,gy,m_guideTab,guideRows().size());
        if (row>=0) { m_dashGuideRow=row;dashGuideActivate();return true; }
        if (gy>=523 && gy<557 && x>=body.x && x<body.x+body.w) {
            const float local=x-body.x;
            if (local<108) dashGuideActivate();
            else if(local<204) guideBack();
            else if(local<340) {
                if(m_guideTab==guide360::Friends) openDiscord(true);
                else {closeGuide(false);openUsers();}
            }
            else { closeGuide(false);dashTab(1); }
            return true;
        }
        if (x<guide360::left || x>guide360::right || gy<146 || gy>560) closeGuide(true);
        return true;
    }
    if (y<85 && x>1050) { openUsers(); return true; }
    if (x<290 && y<265) {
        for (int i=0;i<8;++i) {
            const float cy=218.f+(i-m_dashTab)*35.f;
            if (cy>=50 && y>=cy-16 && y<cy+16) { dashTab(i); break; }
        }
        return true;
    }
    // Hit-test in front-to-back order, matching the visible trapezoids.
    if (m_dashIn>=0.95f) {
        const int front=std::clamp(static_cast<int>(std::round(m_dashCard)),0,std::max(0,dashCount()-1));
        for (int distance=0;distance<7;++distance) for (int side:{1,-1}) {
            if (distance==0 && side<0) continue;
            const int i=front+distance*side;
            if (i<0 || i>=dashCount()) continue;
            if (nxe::panel(i-m_dashCard).contains(x,y)) {
                if (i==m_dashTile) dashActivate();
                else dashMove(i-m_dashTile);
                return true;
            }
        }
    }
    if (y>640 && x<200) dashActivate();
    else if (y>640 && x>=240 && x<360) openGuide();
    return true;
}

void CardsActivity::drawDashBoot(nxui::Renderer& r) {
    m_bootClip.draw(r);
}

void CardsActivity::drawDashboard(nxui::Renderer& r) {
    const bool wallpaper=m_wallTex && m_wallTex->valid() && !m_settings.wallpaper.empty();
    const nxui::Color green=nxeColor({0.29f,0.62f,0.02f,1}), dark=wallpaper?kWhite:nxui::Color(0.17f,0.20f,0.16f,1);
    if(wallpaper){
        drawCover(r,m_wallTex.get(),{0,0,1280,720},0,kWhite);
        // Keep navigation readable while leaving the wallpaper's own horizon intact.
        r.drawGradientRect({0,0,1280,245},{0,0,0,.30f},{0,0,0,0});
        r.drawGradientRect({0,600,1280,120},{0,0,0,.10f},{0,0,0,.68f});
    }else{
    r.drawGradientRect({0,0,1280,492},nxeColor({0.24f,0.47f,0.08f,1}),nxeColor({0.79f,0.85f,0.72f,1}));
    // Soft circular green motif and a bright reflective horizon.
    for (int ring=0;ring<6;++ring) {
        const float radius=520.f+ring*44.f;
        for (int segment=0;segment<48;++segment) {
            const float a=segment*3.14159265f/48.f, b=(segment+1)*3.14159265f/48.f;
            r.drawLine({970+std::cos(a)*radius,-170+std::sin(a)*radius*0.70f},
                       {970+std::cos(b)*radius,-170+std::sin(b)*radius*0.70f},
                       nxeColor({0.92f,1,0.79f,0.028f}),13.f);
        }
    }
    r.drawGradientRect({0,492,1280,228},nxeColor({0.92f,0.94f,0.89f,1}),nxeColor({0.56f,0.61f,0.54f,1}));
    }
    const float p=m_settings.motion?nxe::ease(m_dashIn):1.f;
    for (int i=0;i<8;++i) {
        const float before=218+(i-m_dashPrevTab)*35.f;
        const float after=218+(i-m_dashTab)*35.f;
        const float y=before+(after-before)*p;
        if (y<52 || y>245) continue;
        const float focus=(i==m_dashTab?p:0.f)+(i==m_dashPrevTab?1.f-p:0.f);
        textMid(r,m_fDashNav,nxe::channels[i],76,y,{1,1,1,0.52f+0.48f*focus},0.64f+0.40f*focus);
    }
    if (m_avatar && m_avatar->valid()) r.drawTexture(m_avatar.get(),{1138,40,42,42});
    textMid(r,m_fBody,m_clock,1050,63,kWhite,0.85f);
    if (dashLibrary()) m_dashTile=std::clamp(sel(),0,std::max(0,dashCount()-1));
    m_dashTile=std::clamp(m_dashTile,0,std::max(0,dashCount()-1));

    auto row=[&](int tab,int selected,float scroll,int page,float offset,float opacity) {
        if (opacity<=0.01f) return;
        const bool library=tab==3 || tab==4 || tab==6;
        const int count=library?static_cast<int>(m_pages[page].size()):tab==1?8:tab==2?4:tab==0?2:tab==5?3:10;
        if (!count) { textMid(r,m_fLabel,"Your library is empty",310,410+offset,dark.withAlpha(opacity)); return; }
        static const char* homeLabels[]={"Play Game","My Profile","Game Library","Video Library","Music Library","Picture Library","Homebrew Library","System Settings"};
        static const char* homeIcons[]={"play","sys_mii","pad","play","music","sys_album","dots","gear"};
        static const char* settings[]={"System","Preferences","Audio","Network","Controllers","Storage","Home","Notifications","About","Turn Off"};
        static const char* settingIcons[]={"gear","sun","speaker","wifi","pad","copy","home","bell","dots","power"};
        static const char* social[]={"Friends","My Profile","Discord","Notifications"};
        static const char* socialIcons[]={"friends","sys_mii","discord","bell"};
        static const char* music[]={"Music Library","Audio Settings","Media Apps"};
        static const char* musicIcons[]={"music","speaker","play"};
        auto card=[&](int i) {
            auto panel=nxe::panel(i-scroll,offset);
            if (panel.x+panel.w<0 || panel.x>1280 || panel.y>720 || panel.y+panel.h<0) return;
            std::string label;
            const char* glyph="home";
            const Game* game=nullptr;
            if (library) { game=&m_pages[page][i]; label=game->name; glyph=tab==6?"dots":"pad"; }
            else if (tab==1) {
                label=homeLabels[i]; glyph=homeIcons[i];
                if (i==0) { game=current(); if (game) label=game->name; }
            } else if (tab==2) { label=social[i]; glyph=socialIcons[i]; }
            else if (tab==5) { label=music[i]; glyph=musicIcons[i]; }
            else if (tab==0) { label=i==0?"Search Games & Apps":"Web Browser"; glyph=i==0?"tag":"sys_browser"; }
            else { label=settings[i]; glyph=settingIcons[i]; }
            const nxui::Texture* tex=nullptr;
            if (game) {
                const auto art=m_art.find(game->id);
                if (art!=m_art.end()) {
                    if (art->second.card) tex=art->second.card.get();
                    else if (art->second.icon) tex=art->second.icon.get();
                }
                if (!tex) { const auto th=m_thumbs.find(game->id); if (th!=m_thumbs.end()) tex=th->second.tex.get(); }
            }
            const bool chosen=i==selected;
            const float bright=chosen?1.f:0.78f;
            const float cropW=tex?std::min<float>(tex->width(),tex->height()*panel.w/panel.h):0;
            const float cropH=tex?std::min<float>(tex->height(),tex->width()*panel.h/panel.w):0;
            // Narrow textured strips project the artwork onto a trapezoid using
            // HomeNX's existing renderer; no framebuffer readbacks per frame.
            const int strips=panel.skew>0.01f?24:1;
            auto eachStrip=[&](auto paint) {
                for (int strip=0;strip<strips;++strip) {
                    const float u=static_cast<float>(strip)/strips;
                    paint(u,panel.x+u*panel.w,panel.top(u),panel.height(u),panel.w/strips+0.5f);
                }
            };
            // Group solid and textured passes so one card's artwork stays in
            // a single GPU batch instead of rebinding for every strip.
            eachStrip([&](float,float x,float y,float h,float w) {
                r.drawRect({x+5,y+9,w,h},{0,0,0,0.17f*opacity});
                r.drawGradientRect({x,y,w,h},nxeColor({0.52f*bright,0.77f*bright,0.12f,opacity}),green.withAlpha(opacity));
                if (offset==0.f) r.drawGradientRect({x,y+h+8,w,48},green.withAlpha(0.12f*opacity),green.withAlpha(0));
            });
            if (tex && tex->valid()) eachStrip([&](float u,float x,float y,float h,float w) {
                const nxui::Rect source{(tex->width()-cropW)/2+u*cropW,(tex->height()-cropH)/2,cropW/strips,cropH};
                r.drawTextureSub(tex,source,{x,y,w,h},nxui::Color(bright,bright,bright,opacity));
                if (offset==0.f) r.drawTextureSub(tex,{source.x,source.y+source.height,source.width,-source.height},
                    {x,y+h+8,w,48},{bright,bright,bright,0.10f*opacity});
            });
            eachStrip([&](float u,float x,float y,float h,float w) {
                r.drawGradientRect({x,y+h-63,w,63},{0,0,0,0.03f*opacity},{0,0,0,0.72f*opacity});
                if (chosen && (u==0.f || u>1.f-1.5f/strips)) r.drawRect({x,y,2,h},kWhite.withAlpha(0.8f*opacity));
            });
            if (!tex) {
                const float cy=panel.y+panel.h*0.44f;
                if ((tab==1 || tab==2) && i==1 && m_avatar && m_avatar->valid())
                    r.drawTexture(m_avatar.get(),{panel.x+panel.w/2-52,cy-52,104,104},kWhite.withAlpha(opacity));
                else icon(r,glyph,panel.x+panel.w/2,cy,chosen?88:52,kWhite.withAlpha(opacity));
            }
            textMid(r,m_fBody,fit(m_fBody,label,panel.w-24,chosen?1.05f:0.80f),panel.x+12,panel.y+panel.h-27,kWhite.withAlpha(opacity),chosen?1.05f:0.80f);
            if (chosen) r.drawRect({panel.x,panel.y,panel.w,2},kWhite.withAlpha(opacity));
        };
        const int front=std::clamp(static_cast<int>(std::round(scroll)),0,count-1);
        // Furthest right first, then left, then the frontmost card.
        for (int i=std::min(count-1,front+6);i>front;--i) card(i);
        for (int i=std::max(0,front-2);i<front;++i) card(i);
        card(front);
    };
    r.pushClipRect({0,246,1280,385});
    if (p<1.f) row(m_dashPrevTab,m_dashPrevTile,m_dashPrevCard,m_dashPrevPage,-m_dashDir*490.f*p,1.f-p);
    row(m_dashTab,m_dashTile,m_dashCard,m_page,m_dashDir*490.f*(1.f-p),0.35f+0.65f*p);
    r.popClipRect();
    const int count=dashCount();
    textMid(r,m_fBody,std::to_string(count?m_dashTile+1:0)+" of "+std::to_string(count),302,625,dark,0.85f);
    r.drawCircle({87,672},12,{0.29f,0.62f,0.02f,1});
    textMid(r,m_fBadge,"A",81,672,kWhite,0.85f);
    textMid(r,m_fBody,"Select",108,672,dark,0.9f);
    textMid(r,m_fBody,"Y  Guide    B  My Xbox    ZL / ZR  Channels",264,672,dark,0.8f);
    textMid(r,m_fBody,"HomeNX 360",1050,672,dark,0.9f);
    if (m_dashGuide) drawGuide(r);
    if (m_leaving) r.drawRect({0,0,1280,720},{0,0,0,std::min(1.f,m_leaveT*1.5f)});
}
