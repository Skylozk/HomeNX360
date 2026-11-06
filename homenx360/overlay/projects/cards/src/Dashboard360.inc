// Included inside namespace cards, after CardsActivity's drawing helpers.
bool CardsActivity::dashLibrary() const {
    return m_dashTab == 3 || m_dashTab == 4 || m_dashTab == 6;
}

int CardsActivity::dashCount() const {
    if (m_dashTab == 1) return 9;
    if (dashLibrary()) {
        const int count = static_cast<int>(m_pages[m_page].size());
        return std::min(10, count - std::min((m_sels[m_page] / 10) * 10, count));
    }
    if (m_dashTab == 0) return 2;
    if (m_dashTab == 2) return 4;
    if (m_dashTab == 5) return 3;
    return 10;
}

void CardsActivity::dashTab(int tab) {
    m_dashTab = dash360::wrapTab(tab);
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
    if (dashLibrary()) m_dashTile = sel() % 10;
    m_audio.play("navigation");
}

void CardsActivity::dashSettings(const std::string& section) {
    openSettings();
    for (int i = 0; i < static_cast<int>(m_sheet.size()); ++i)
        if (m_sheet[i].name == section) { m_sheetSec = i; m_sheetRow = 0; break; }
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
                        m_dashTile = sel() % 10;
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
            case 0: case 3: if (current()) play(); else dashTab(3); break;
            case 1: openPagePicker(); break;
            case 2: dashTab(3); sortGames(0); select(0); m_dashTile = 0; break;
            case 4: dashSettings("Bluetooth"); break;
            case 5: openGallery(); break;
            case 6: dashTab(3); break;
            case 7: dashTab(6); break;
            case 8:
#ifdef SWITCHU_MENU
                openNews();
#else
                say("News is available when this dashboard runs as HOME.");
#endif
                break;
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

bool CardsActivity::dashInput(float dt) {
    if (m_moving) return false;
    auto& in = app().input();
    using B = nxui::Button;
    if (in.isDown(B::L) || in.isDown(B::ZL)) dashTab(m_dashTab - 1);
    if (in.isDown(B::R)) dashTab(m_dashTab + 1);
    if (in.isDown(B::ZR)) openQuick();
    if (in.isDown(B::Minus)) openNotifications();
    if (in.isDown(B::X)) askClose();
    if (in.isDown(B::Y)) {
        openMenu("Guide", {
            action("Home", [this]() { dashTab(1); m_dashNav = false; }),
            action("Search", [this]() { dashSearch(); }),
            action("My pages / pins", [this]() { openPagePicker(); }),
            action("Quick settings", [this]() { openQuick(); }),
            action("Settings", [this]() { openSettings(); }),
            action("Sleep / power", [this]() { openPower(); })
        });
        return true;
    }
    if (in.isDown(B::Plus) && current() && (dashLibrary() || (m_dashTab == 1 && (m_dashTile == 0 || m_dashTile == 3)))) { openOptions(); return true; }
    if (in.isDown(B::B)) { dashTab(1); m_dashNav = false; return true; }
    int dx = 0, dy = 0;
    const B directions[][3] = {{B::DLeft,B::LStickL,B::RStickL}, {B::DRight,B::LStickR,B::RStickR},
                              {B::DUp,B::LStickU,B::RStickU}, {B::DDown,B::LStickD,B::RStickD}};
    bool held = false, down = false;
    int direction = -1;
    for (int i = 0; i < 4; ++i)
        for (auto button : directions[i]) {
            if (in.isDown(button)) { down = true; direction = i; }
            if (in.isHeld(button)) { held = true; if (direction < 0) direction = i; }
        }
    if (down) m_dashRepeat = 0.32f;
    else if (held) { m_dashRepeat -= dt; if (m_dashRepeat <= 0) { down = true; m_dashRepeat = 0.12f; } }
    if (down) {
        dx = direction == 0 ? -1 : direction == 1 ? 1 : 0;
        dy = direction == 2 ? -1 : direction == 3 ? 1 : 0;
        if (m_dashNav) {
            if (dx) dashTab(m_dashTab + dx);
            if (dy > 0) m_dashNav = false;
        } else if (dashCount() > 0) {
            const int old = std::clamp(m_dashTile, 0, dashCount()-1);
            const int next = dash360::neighbor(old, dashCount(), m_dashTab == 1, dx, dy);
            if (dashLibrary() && ((dx > 0 && old % 5 == 4) || (dx < 0 && old % 5 == 0))) {
                const int target = sel() + dx;
                if (target >= 0 && target < static_cast<int>(games().size())) {
                    select(target); m_dashTile = sel() % 10;
                }
            } else if (next != old) {
                m_dashTile = next;
                if (dashLibrary()) select((sel()/10)*10 + next);
            } else if (dy < 0) m_dashNav = true;
            m_audio.play("navigation");
        } else if (dy < 0) m_dashNav = true;
    }
    if (in.isDown(B::A)) {
        if (m_dashNav) m_dashNav = false;
        else dashActivate();
    }
    return true;
}

bool CardsActivity::dashTouch() {
    // Existing modal screens keep their touch handlers.
    if (m_pickOpen || m_spQrOpen || m_sheetOpen || m_notifOpen || m_quickOpen || m_powerOpen || m_dotsOpen
        || m_musicOpen || m_optOpen || !m_menu.empty() || m_moving)
        return handleTouch();
    auto& in = app().input();
    if (!in.touchUp()) return in.isTouching();
    const float x = in.touchX(), y = in.touchY();
    if (y >= 116 && y < 171) {
        for (int i = 7; i >= 0; --i)
            if (x >= dash360::tabX[i] - 12) { dashTab(i); m_dashNav = false; break; }
        return true;
    }
    if (dashLibrary() && y >= 590 && y < 640) {
        const int base = (sel()/10)*10;
        if (x >= 970 && base+10 < static_cast<int>(games().size())) select(base+10);
        else if (x >= 870 && x < 970 && base >= 10) select(base-10);
        m_dashTile = sel()%10;
        return true;
    }
    if (y < 100 && x > 1100) { openUsers(); return true; }
    for (int i = 0; i < dashCount(); ++i)
        if (dash360::tile(i, m_dashTab == 1).contains(x,y)) {
            m_dashNav = false;
            m_dashTile = i;
            if (dashLibrary()) select((sel()/10)*10+i);
            dashActivate();
            return true;
        }
    return true;
}

void CardsActivity::drawDashboard(nxui::Renderer& r) {
    const nxui::Color green(0.f,0.52f,0.f,1.f), pale(0.67f,0.67f,0.67f,1.f);
    r.drawGradientRect({0,0,1280,610}, {0.22f,0.22f,0.22f,1}, {0.72f,0.72f,0.72f,1});
    r.drawGradientRect({0,610,1280,110}, {0.72f,0.72f,0.72f,1}, {0.50f,0.50f,0.50f,1});
    for (int i = 0; i < 8; ++i) {
        const float scale = i == 4 ? 0.93f : 1.f;
        textMid(r, m_fDashNav, dash360::tabs[i], dash360::tabX[i], 143,
                i == m_dashTab ? kWhite : nxui::Color(0.58f,0.58f,0.58f,1), scale);
        if (m_dashNav && i == m_dashTab)
            r.drawRect({dash360::tabX[i],162,measured(m_fDashNav,dash360::tabs[i]).x*scale,2}, kWhite);
    }
    if (m_avatar && m_avatar->valid()) r.drawTexture(m_avatar.get(), {1154,40,28,34});
    else icon(r,"friends",1168,57,28,kWhite);
    textMid(r,m_fBody,m_clock,1110,93,pale,0.8f);
    // Neighboring hubs peeking in at the edges, as in the reference.
    r.drawGradientRect({-118,246,174,336},green.withAlpha(0.45f),green.withAlpha(0.12f));
    r.drawGradientRect({1212,246,174,336},green.withAlpha(0.45f),green.withAlpha(0.12f));
    textMid(r,m_fBody,m_dashTab == 2 ? "games" : "Friends",1228,296,kWhite.withAlpha(0.35f));

    const bool library = dashLibrary();
    if (library) m_dashTile = sel()%10;
    const int count = dashCount();
    m_dashTile = std::clamp(m_dashTile,0,std::max(0,count-1));
    static const char* homeLabels[] = {"Play", "My Pins", "Recent", "Play", "Controllers", "Album", "My Games", "My Apps", "News"};
    static const char* homeIcons[] = {"play","tag","repeat","play","pad","sys_album","home","dots","news"};
    static const char* settings[] = {"System", "Preferences", "Audio", "Network", "Controllers", "Storage", "Home", "Notifications", "About", "Turn Off"};
    static const char* settingIcons[] = {"gear","sun","speaker","wifi","pad","copy","home","bell","dots","power"};
    static const char* social[] = {"Friends", "Profile", "Discord", "Notifications"};
    static const char* socialIcons[] = {"friends","sys_mii","discord","bell"};
    static const char* music[] = {"Music Player", "Audio Settings", "Media Apps"};
    static const char* musicIcons[] = {"music","speaker","play"};
    for (int i = 0; i < count; ++i) {
        auto t = dash360::tile(i,m_dashTab == 1);
        const nxui::Rect box{t.x,t.y,t.w,t.h};
        const bool chosen = i == m_dashTile && !m_dashNav;
        r.drawRect({t.x+2,t.y+5,t.w,t.h},nxui::Color(0,0,0,0.15f));
        r.drawGradientRect(box,green, nxui::Color(0.f,0.43f,0.f,1));
        std::string label;
        const char* glyph = "home";
        const Game* game = nullptr;
        if (library) {
            const int index = (sel()/10)*10+i;
            if (index < static_cast<int>(games().size())) game = &games()[index];
            label = game ? game->name : "";
            glyph = m_dashTab == 6 ? "dots" : m_dashTab == 4 ? "play" : "pad";
        } else if (m_dashTab == 1) {
            label = homeLabels[i]; glyph = homeIcons[i];
            if (i == 0 || i == 3) { game = current(); if (game) label = (i == 0 ? "Play " : "") + game->name; }
        } else if (m_dashTab == 2) { label = social[i]; glyph = socialIcons[i]; }
        else if (m_dashTab == 5) { label = music[i]; glyph = musicIcons[i]; }
        else if (m_dashTab == 0) { label = i == 0 ? "Search Games & Apps" : "Web Browser"; glyph = i == 0 ? "tag" : "sys_browser"; }
        else { label = settings[i]; glyph = settingIcons[i]; }
        const nxui::Texture* tex = nullptr;
        if (game) {
            const auto art = m_art.find(game->id);
            if (art != m_art.end()) {
                if (m_dashTab == 1 && i == 3 && art->second.bg) tex = art->second.bg.get();
                else if (art->second.card) tex = art->second.card.get();
                else if (art->second.icon) tex = art->second.icon.get();
            }
            if (!tex) { const auto th = m_thumbs.find(game->id); if (th != m_thumbs.end() && th->second.tex) tex = th->second.tex.get(); }
        }
        if (tex && tex->valid()) {
            drawCover(r,tex,box,0,kWhite,0,0);
            r.drawGradientRect({t.x,t.y+t.h-48,t.w,48},nxui::Color(0,0,0,0),nxui::Color(0,0,0,0.9f));
        } else {
            const float size = t.h < 100 ? 34 : m_dashTab == 1 && i == 3 ? 88 : 62;
            const float cx = t.x+t.w/2, cy = t.y+(t.h-24)/2;
            if (m_dashTab == 1 && i == 2) {
                r.drawCircle({cx,cy},26,kWhite);
                r.drawCircle({cx,cy},21,green);
                r.drawLine({cx,cy},{cx,cy-14},kWhite,4);
                r.drawLine({cx,cy},{cx+12,cy},kWhite,4);
            } else if (m_dashTab == 1 && i == 1) {
                // The diagonal white pushpin used by the reference dashboard.
                r.drawTriangle({cx+5,cy-24},{cx+24,cy-5},{cx+8,cy+2},kWhite);
                r.drawTriangle({cx+5,cy-24},{cx+8,cy+2},{cx-2,cy-8},kWhite);
                r.drawTriangle({cx-2,cy-8},{cx+8,cy+2},{cx-9,cy+18},kWhite);
                r.drawTriangle({cx-13,cy-2},{cx+2,cy+13},{cx-9,cy+18},kWhite);
                r.drawLine({cx-9,cy+12},{cx-22,cy+25},kWhite,3);
            } else icon(r,glyph,cx,cy,size,kWhite.withAlpha(0.91f));
        }
        textMid(r,m_fBody,fit(m_fBody,label,t.w-20,0.96f),t.x+10,t.y+t.h-16,kWhite,0.96f);
        if (chosen) {
            r.drawRectOutline({t.x-4,t.y-4,t.w+8,t.h+8},kWhite.withAlpha(0.90f),3);
            r.drawRectOutline(box,nxui::Color(0.40f,0.90f,0.10f,1),2);
        }
        // Faint floor reflection, kept to the last row of the tile group.
        if (t.y+t.h >= 580) {
            r.drawGradientRect({t.x,588,t.w,32},green.withAlpha(0.12f),green.withAlpha(0));
        }
    }
    if (library && count == 0) {
        const std::string message = m_dashTab == 6 ? "No homebrew found in sdmc:/switch" : m_dashTab == 4 ? "No media apps found" : "No games installed";
        textMid(r,m_fLabel,message,228,310,kWhite);
    }
    if (library) {
        textMid(r,m_fBody,"< Prev     Next >",880,610,kWhite.withAlpha(0.8f),0.85f);
        const auto n = games().size();
        const auto page = static_cast<size_t>(sel()/10+1);
        textMid(r,m_fBody,std::to_string(n)+" items    "+std::to_string(page)+" / "+std::to_string(std::max<size_t>(1,(n+9)/10)),228,610,kWhite.withAlpha(0.8f),0.85f);
    }
    r.drawCircle({104,660},9,{0.35f,0.72f,0.1f,1});
    textMid(r,m_fBadge,"A",99,660,kWhite,0.8f);
    textMid(r,m_fBody,"Select",119,660,kWhite,0.85f);
    textMid(r,m_fBody,"L / R  Hubs     Y  Guide     ZR  Quick Settings",228,660,kWhite.withAlpha(0.8f),0.8f);
    if (current() && (library || (m_dashTab == 1 && (m_dashTile == 0 || m_dashTile == 3)))) textMid(r,m_fBody,"+  Options",990,660,kWhite.withAlpha(0.8f),0.8f);
    if (m_moving) textMid(r,m_fBody,"Move: Left / Right     A  Done     B  Cancel",228,628,kWhite);
    if (m_leaving) r.drawRect({0,0,1280,720},nxui::Color(0,0,0,std::min(1.f,m_leaveT*1.5f)));
}
