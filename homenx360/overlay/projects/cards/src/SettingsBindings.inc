// Included inside namespace cards. Original settings callbacks stay authoritative.
void CardsActivity::buildNxeSettings() {
    using namespace settings360;
    auto original=std::move(m_sheet);
    m_sheet.clear();m_sheet.reserve(Count);
    for(const auto* title:titles) m_sheet.push_back({title,"gear",{}});
    auto append=[&](settings360::Page page,const char* section,std::initializer_list<const char*> labels={}) {
        const auto found=std::find_if(original.begin(),original.end(),[&](const Section& s){return s.name==section;});
        if(found==original.end()) return;
        for(const auto& item:found->rows) {
            if(labels.size() && std::none_of(labels.begin(),labels.end(),[&](const char* name){return item.label==name;})) continue;
            m_sheet[page].rows.push_back(item);
        }
    };
    auto link=[&](settings360::Page page,const char* label,settings360::Page target,const char* detail,const char* glyph=nullptr) {
        auto item=action(label,[this,target](){openNxePage(target);});
        item.detail=detail;item.glyph=glyph;m_sheet[page].rows.push_back(std::move(item));
    };
    auto note=[&](settings360::Page page,const char* label,const char* value,const char* detail) {
        auto item=info(label,[value](){return std::string(value);});item.detail=detail;
        m_sheet[page].rows.push_back(std::move(item));
    };
    for(const auto& entry:systemLinks) link(System,entry.label,entry.target,entry.detail,entry.glyph);
    for(const auto& entry:consoleLinks) link(Console,entry.label,entry.target,entry.detail);

    append(Display,"Console",{"Console's TV output","Auto-brightness"});
    append(Display,"Look",{"TV resolution"});
    m_sheet[Display].rows.push_back(slider("Brightness",[this](){return int(std::max(0.f,m_quick.brightness)*100+0.5f);},
        [this](int value){m_quick.brightness=value/100.f;setBrightness(m_quick.brightness);},"Handheld screen brightness. TV brightness is controlled by your television."));
    append(settings360::Audio,"Sound");
    append(settings360::Audio,"Bluetooth",{"Headphones and speakers","Pair headphones or a speaker"});
    link(settings360::Audio,"Bluetooth Devices",Remote,"Connect headphones, speakers, and controllers using the Switch's Bluetooth controls.","bluetooth");
    append(Language,"Console",{"Language"});
    note(Language,"Console region","Nintendo Switch","HomeNX language changes here. Manage console region and Nintendo Account country using Nintendo's own settings.");
    append(Clock,"Look",{"Clock"});
    append(Clock,"Console",{"Set the time automatically","Date","Hour","Minute"});
#ifndef SWITCHU_MENU
    note(Clock,"Console date and time","HOME frontend required","The standalone preview can change the dashboard's clock format. Console date and time editing is available when HomeNX360 runs as HOME.");
#endif
    append(Startup,"Look",{"Startup animation","Sign-in at startup","Preview startup"});
    append(Startup,"Console",{"Sleep in handheld","Sleep on the TV"});
    m_sheet[Startup].rows.push_back(action("Turn Off Console",[this](){m_sheetOpen=false;openPower();}));
    m_sheet[Startup].rows.back().detail="Open the Switch's sleep, shutdown, and restart controls.";
    note(AutoPlay,"Disc auto-play","Unavailable on Nintendo Switch","This console has no Xbox 360 disc drive. Use A on a game card to launch installed software.");
    append(AutoPlay,"Look",{"Launch animation"});
    append(AutoPlay,"Home",{"Ask who's playing","Keep homebrew open"});
    append(Remote,"Bluetooth");
    append(SystemInfo,"About",{"HomeNX360","System version","Atmosphere","Games","Homebrew","Free on the SD card","Terms, privacy, licenses","Performance log","Close HomeNX360"});
    append(Memory,"Storage");
    append(Network,"Internet");
    append(Computers,"Console",{"Connect to a computer"});
    note(Computers,"Connection","USB file transfer (MTP)","Select Connect to a computer, then connect a USB data cable. This uses HomeNX MTP, not Xbox 360 Windows Media Center pairing.");
    note(Vision,"Xbox LIVE Vision","Unavailable on Nintendo Switch","The Xbox 360 camera and Xbox LIVE video chat cannot operate on Nintendo Switch. This page reports that limitation; it does not pretend a camera is connected.");
    note(Family,"Nintendo Parental Controls","Managed by Nintendo","Use Nintendo Switch Parental Controls for console restrictions. HomeNX's lock screen below is a separate feature and does not replace Nintendo parental protection.");
    append(Family,"Home",{"Ask who's playing"});
    link(Family,"Lock Screen",Lock,"Configure HomeNX's lock screen and password.","moon");
    m_sheet[Family].rows.push_back(action("Profiles",[this](){openUsers();}));
    m_sheet[Family].rows.back().detail="Choose the real Nintendo Switch profile used to launch games.";
    m_sheet[Initial].rows.push_back(action("Choose Profile",[this](){openUsers();}));
    for(const auto& entry:consoleLinks) if(entry.target==Language || entry.target==Clock || entry.target==Startup)
        link(Initial,entry.label,entry.target,entry.detail);
    link(Initial,"Network Settings",Network,"Review the current network and run a connection test.","wifi");
    m_sheet[Initial].rows.push_back(action("Done",[this](){backNxeSettings();}));
    m_sheet[Initial].rows.back().detail="Return to System Settings. Your console and saved data are not reset.";
    link(Preferences,"Dashboard",Look,"Change this profile's colour and wallpaper, dashboard transitions, and startup playback.");
    link(Preferences,"Games and Apps",Home,"Manage software, custom pages, sorting, and hidden cards.","pad");
    link(Preferences,"Lock Screen",Lock,"Configure the wake screen, password, pictures, and music.","moon");
    link(Preferences,"Notifications",Notifications,"Configure real HomeNX notifications and playtime milestones.","bell");
    link(Preferences,"Pictures",Pictures,"Change game card artwork and manage downloaded pictures.","sys_album");
    link(Preferences,"Updates",Updates,"Manage HomeNX and homebrew updates. The upstream HomeNX updater replaces this custom dashboard.","download");
    link(Preferences,"System Info",SystemInfo,"View the real system and HomeNX versions.","dots");
    append(Look,"Look",{"Theme profile","Change Theme","Background","Theme colour","Reset profile theme","Launch animation","Animations","Startup animation","Preview startup"});
    append(Home,"Home",{"Custom pages","Sort games","Ask who's playing","Keep homebrew open","Look for new software","Hidden cards","Reset my order"});
    append(Lock,"Lock screen");
    append(Notifications,"Notifications");
    append(Pictures,"Pictures");
    append(Updates,"Updates");
    // Each control preserves its original callback, getter and save behavior.
    // Do not move or reorder this fixed page vector: sheetBack stores its index.
}

