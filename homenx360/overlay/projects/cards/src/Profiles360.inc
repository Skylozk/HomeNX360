// Included inside namespace cards. Profile names and launch identities come
// from Nintendo accounts; full catalog appearances are app-local, keyed by full UID.
namespace {
std::string profile360Id(AccountUidLike uid){
    char text[33];std::snprintf(text,sizeof(text),"%016llX%016llX",
        static_cast<unsigned long long>(uid.uid[0]),static_cast<unsigned long long>(uid.uid[1]));return text;
}
}
void CardsActivity::openUsers(){openProfiles();}

void CardsActivity::openProfiles(bool startup){
    m_profiles.begin(startup);m_profilePending=false;m_profileRepeat=0;m_profileLoadTime=0;m_profileAge=0;m_profileAssetsFailed=false;
    m_dashGuide=false;m_guideClosing=false;m_menu.clear();m_profileUsers.clear();
    forgetAllArt(); // make room for the character textures while this screen covers the library
    loadProfiles();
    if(!m_profileAssets.ready && !m_profileAssetJob.valid()){
        const std::string base=std::string(kAssets)+"avatars/";
        m_profileAssetJob=std::async(std::launch::async,[base](){
            offScreenCore();avatar360::AssetData data;
            try{
                for(size_t i=0;i<data.models.size();++i)avatar360::loadModel(base+avatar360::modelFiles[i]+".avb",data.models[i]);
                data.catalog.load("sdmc:/switch/SwitchU/cards/avatars/catalog");
            }catch(...){data={};}
            threadDone();return data;
        });
    }
    m_audio.play("show_modal");
}
void CardsActivity::loadProfiles(){
    if(m_profileJob.valid())return;
    m_profileJob=std::async(std::launch::async,[](){offScreenCore();auto users=listUsers();threadDone();return users;});
}
void CardsActivity::confirmProfile(){
    const auto picked=m_profiles.signIn();if(!picked)return;
    const auto user=std::find_if(m_profileUsers.begin(),m_profileUsers.end(),[&](const auto& u){return profile360Id(u.uid)==picked->id;});
    if(user==m_profileUsers.end())return;
    AccountUid uid{};uid.uid[0]=user->uid.uid[0];uid.uid[1]=user->uid.uid[1];AccountProfile profile{};
    if(R_FAILED(accountGetProfile(&profile,uid))){
        say("That profile is no longer on the console.");m_profiles.begin(false);loadProfiles();return;
    }
    accountProfileClose(&profile);
    setUser(user->uid);m_guideName=user->name;m_profiles.signedIn();m_audio.play("activation");
}
void CardsActivity::saveProfileAvatar(){
    if(m_profiles.draft.expanded){
        auto* preview=m_profileAssets.request(m_profiles.draft);
        if(!preview || !preview->avatar.model.valid() || !preview->avatar.error.empty()){say(preview && !preview->avatar.error.empty()?preview->avatar.error:"Wait for the outfit to finish loading.");return;}
    }
    // Revalidate the target account before persisting; editing another
    // profile's appearance must never silently change the active account.
    const auto user=std::find_if(m_profileUsers.begin(),m_profileUsers.end(),[&](const auto& u){return profile360Id(u.uid)==m_profiles.editingId;});
    if(user==m_profileUsers.end())return;
    AccountUid uid{};uid.uid[0]=user->uid.uid[0];uid.uid[1]=user->uid.uid[1];AccountProfile profile{};
    if(R_FAILED(accountGetProfile(&profile,uid))){say("That profile is no longer on the console.");m_profiles.back();return;}
    accountProfileClose(&profile);
    if(m_profiles.save(m_settings.profileAvatars)){m_settings.save();m_audio.play("toggle_on");}
}
void CardsActivity::pollProfileJobs(){
    if(m_profileJob.valid() && m_profileJob.wait_for(std::chrono::seconds(0))==std::future_status::ready){
        try{m_profileUsers=m_profileJob.get();}catch(...){m_profileUsers.clear();}
        std::vector<avatar360::Profile> profiles;for(const auto& u:m_profileUsers)profiles.push_back({profile360Id(u.uid),u.name});
        m_profiles.loaded(std::move(profiles),profile360Id(m_user));
        m_profileScroll=float(avatar360::firstVisible(m_profiles.selected,int(m_profiles.profiles.size())));
    }
    if(m_profileAssetJob.valid() && m_profileAssetJob.wait_for(std::chrono::seconds(0))==std::future_status::ready){
        try{auto data=m_profileAssetJob.get();m_profileAssets.models=std::move(data.models);m_profileAssets.catalog=std::move(data.catalog);}catch(...){m_profileAssets.models={};}
        m_profileAssetsFailed=false;for(const auto& model:m_profileAssets.models)m_profileAssetsFailed|=!model.valid();
        m_profileAssets.ready=!m_profileAssetsFailed;
    }
}
void CardsActivity::updateProfiles(float dt){
    using S=avatar360::Screen;using B=nxui::Button;auto& input=app().input();
    m_profileAge+=std::max(0.f,dt);
    pollProfileJobs();
    m_profileAssets.pump(app().gpu(),app().renderer());
    if(!m_profileAssetsFailed && m_profileAssets.ready && !m_profileAssets.available()){
        m_profileAssets.upload(app().gpu(),app().renderer());m_profileLoadTime+=dt;
        if(m_profileLoadTime>15.f)m_profileAssetsFailed=true;
    }
    auto editAvatar=[&](){
        if(!m_profileAssets.catalog.ready){say("Install the complete update ZIP to use the expanded avatar catalog.");return;}
        m_profiles.edit(m_settings.profileAvatars);m_profileAssets.catalog.initialize(m_profiles.draft);
    };
    auto randomizeAvatar=[&](){
        auto& look=m_profiles.draft;uint32_t seed=uint32_t(armGetSystemTick());
        auto next=[&](){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;};if(!seed)seed=0x360360;
        for(int i=0;i<12;++i)look.colors[i]=next()%avatar360::colorCount(i);
        m_profileAssets.catalog.initialize(look);
        for(int g=0;g<21;++g){auto options=m_profileAssets.catalog.options(g,look.colors[0]);if(!options.empty()){const auto* item=options[next()%options.size()];look.items[g]=item?item->key:"";}}
    };
    const auto previous=m_profiles.screen;
    auto close=[&](){m_profiles.screen=S::Closed;};
    const bool footerTouch=input.touchUp() && input.touchY()>=643;
    const bool waiting=m_profiles.screen==S::Loading || !m_profileAssets.available();
    if(input.isDown(B::B) || (waiting && footerTouch && input.touchX()<210))m_profiles.back();
    else if((input.isDown(B::Y) || (m_profileAssetsFailed && footerTouch && input.touchX()>=250 && input.touchX()<540)) && m_profiles.screen==S::Picker){close();openUserTools();}
    else if(m_profiles.screen==S::Welcome){
        if(input.isDown(B::A) || (footerTouch && input.touchX()<270))close();else m_profiles.tick(dt);
    }else if(m_profiles.screen==S::Picker && m_profileAssets.available()){
        const bool left=input.isDown(B::DLeft)||input.isDown(B::LStickL),right=input.isDown(B::DRight)||input.isDown(B::LStickR);
        if(left||right){m_profiles.move(right?1:-1);m_audio.play("navigation");}
        if(input.isDown(B::A))confirmProfile();
        else if(input.isDown(B::X))editAvatar();
        if(input.touchUp()){
            const float x=input.touchX(),y=input.touchY();
            if(std::abs(input.touchDeltaX())>60)m_profiles.move(input.touchDeltaX()<0?1:-1);
            else if(y>=643){
                if(x<200)confirmProfile();else if(x<390)m_profiles.back();else if(x<640)editAvatar();else{close();openUserTools();}
            }else for(int i=0;i<int(m_profiles.profiles.size());++i)if(avatar360::profileBox(i,m_profileScroll).contains(x,y)){
                if(i==m_profiles.selected)confirmProfile();else m_profiles.selected=i;break;
            }
        }
    }else if(m_profiles.screen==S::Editor){
        int vertical=0,horizontal=0;
        const bool up=input.isDown(B::DUp)||input.isDown(B::LStickU),down=input.isDown(B::DDown)||input.isDown(B::LStickD);
        const bool left=input.isDown(B::DLeft)||input.isDown(B::LStickL),right=input.isDown(B::DRight)||input.isDown(B::LStickR);
        const bool held=input.isHeld(B::DLeft)||input.isHeld(B::DRight)||input.isHeld(B::DUp)||input.isHeld(B::DDown)||input.isHeld(B::LStickL)||input.isHeld(B::LStickR)||input.isHeld(B::LStickU)||input.isHeld(B::LStickD);
        if(up||down||left||right){vertical=down-up;horizontal=right-left;m_profileRepeat=.4f;}
        else if(held && (m_profileRepeat-=dt)<=0){m_profileRepeat=.14f;vertical=(input.isHeld(B::DDown)||input.isHeld(B::LStickD))-(input.isHeld(B::DUp)||input.isHeld(B::LStickU));horizontal=(input.isHeld(B::DRight)||input.isHeld(B::LStickR))-(input.isHeld(B::DLeft)||input.isHeld(B::LStickL));}
        else if(!held)m_profileRepeat=0;
        m_profiles.field=std::clamp(m_profiles.field+vertical,0,int(avatar360::CFieldCount)-1);
        if(input.isDown(B::ZL))horizontal=-10;
        if(input.isDown(B::ZR))horizontal=10;
        if(horizontal)m_profileAssets.catalog.cycle(m_profiles.draft,m_profiles.field,horizontal);
        if(input.isHeld(B::L))m_profiles.yaw-=dt*1.8f;
        if(input.isHeld(B::R))m_profiles.yaw+=dt*1.8f;
        if(input.isDown(B::A))m_profileAssets.catalog.cycle(m_profiles.draft,m_profiles.field,1);
        if(input.isDown(B::Y))randomizeAvatar();
        if(input.isDown(B::X))saveProfileAvatar();
        if(input.touchUp()){
            const float x=input.touchX(),y=input.touchY();
            if(y>=643){if(x<230)saveProfileAvatar();else if(x<430)m_profiles.back();else if(x<690)randomizeAvatar();}
            else if(x<580 && std::abs(input.touchDeltaX())>12)m_profiles.yaw+=input.touchDeltaX()*.009f;
            else {
                const int top=std::clamp(m_profiles.field-8,0,int(avatar360::CFieldCount)-9);
                for(int i=top;i<std::min(top+9,int(avatar360::CFieldCount));++i)if(avatar360::editorRow(i,top).contains(x,y)){m_profiles.field=i;m_profileAssets.catalog.cycle(m_profiles.draft,i,x<900?-1:1);break;}
            }
        }
    }
    const float target=avatar360::firstVisible(m_profiles.selected,int(m_profiles.profiles.size()));
    m_profileScroll+=(target-m_profileScroll)*(m_settings.motion?std::min(1.f,dt*12.f):1.f);
    if(previous!=S::Closed && !m_profiles.open()){m_profileAssets.releaseTextures();wantArt();}
}
void CardsActivity::drawCharacter(nxui::Renderer& renderer,const avatar360::Look& look,float x,float y,float scale,float yaw,bool wave,bool portrait,float alpha){
    const bool drawn=avatar360::drawCharacter(m_profileAssets,renderer,look,x,y,scale,yaw,
        m_settings.motion?(wave?m_profiles.welcomeTime:m_profileAge+look.value[avatar360::Character]*1.3f):0.f,alpha,wave,portrait);
    if(!drawn && look.expanded){
        const auto* entry=m_profileAssets.request(look);const std::string message=!m_profileAssets.catalog.ready?"Install the complete avatar catalog.":entry && !entry->avatar.error.empty()?entry->avatar.error:"Loading outfit...";
        textMid(renderer,m_fBody,fit(m_fBody,message,450,.7f),x-110,y-170,{.2f,.23f,.22f,1},.7f);
    }
}
void CardsActivity::drawProfiles(nxui::Renderer& r){
    using S=avatar360::Screen;if(!m_profiles.open())return;
    const nxui::Color white{1,1,1,1},dark{.15f,.17f,.16f,1},muted{.34f,.36f,.35f,1},green{.36f,.66f,.06f,1};
    r.drawGradientRect({0,0,1280,405},{.18f,.22f,.22f,1},{.56f,.58f,.57f,1});
    r.drawGradientRect({0,405,1280,315},{.56f,.58f,.57f,1},{.88f,.89f,.88f,1});
    auto right=[&](const std::string& text,float y,float scale){textMid(r,m_fDashNav,text,1180-measured(m_fDashNav,text).x*scale,y,white,scale);};
    auto centered=[&](nxui::Font& font,const std::string& text,float x,float y,nxui::Color color,float scale=1.f){textMid(r,font,text,x-measured(font,text).x*scale/2,y,color,scale);};
    const bool edit=m_profiles.screen==S::Editor,welcome=m_profiles.screen==S::Welcome;
    right(edit?"customize avatar":welcome?"welcome":"sign in or out",75,1.05f);
    right(edit?"Make this avatar yours":welcome?"Signed in":"Choose your profile",111,.48f);
    auto footer=[&](float x,const char* letter,const char* label,nxui::Color color){r.drawCircle({x,666},11,color);centered(m_fBadge,letter,x,666,white,.82f);textMid(r,m_fBody,label,x+19,666,dark,.88f);};
    if(m_profiles.screen==S::Loading || (!m_profileAssets.available() && !m_profileAssetsFailed)){
        centered(m_fDashNav,"Loading avatars...",640,370,white,.8f);
        footer(88,"B","Back",{.75f,.08f,.07f,1});return;
    }
    if(m_profileAssetsFailed){
        centered(m_fBody,"Avatar files could not be loaded.",640,330,white);
        centered(m_fBody,"Reinstall the complete HomeNX360 update, including the avatars folder.",640,370,white,.85f);
        footer(88,"B","Back",{.75f,.08f,.07f,1});footer(280,"Y","Manage users",{.84f,.66f,.04f,1});return;
    }
    if(edit){
        const auto* profile=m_profiles.current();
        drawCharacter(r,m_profiles.draft,330,575,310,m_profiles.yaw);
        if(profile)centered(m_fLabel,fit(m_fLabel,profile->name,440),330,606,dark,1.1f);
        const int top=std::clamp(m_profiles.field-8,0,int(avatar360::CFieldCount)-9);
        for(int i=top;i<std::min(top+9,int(avatar360::CFieldCount));++i){
            const auto row=avatar360::editorRow(i,top);const bool chosen=i==m_profiles.field;
            if(chosen)r.drawGradientRect({row.x,row.y,row.w,row.h},{.49f,.76f,.08f,1},{.28f,.53f,.02f,1});
            textMid(r,m_fBody,avatar360::catalogFieldNames[i],row.x+12,row.y+12,chosen?white:dark,.64f);
            const std::string value="<  "+fit(m_fBody,m_profileAssets.catalog.label(m_profiles.draft,i),390,.72f)+"  >";
            textMid(r,m_fBody,value,row.x+row.w-12-measured(m_fBody,value).x*.72f,row.y+29,chosen?white:dark,.72f);
        }
        r.drawRect({1188,140,3,396},{.35f,.39f,.36f,.35f});
        r.drawRect({1187,140+396.f*top/avatar360::CFieldCount,5,396.f*9/avatar360::CFieldCount},green);
        textMid(r,m_fBody,"L / R  Rotate   ZL / ZR  Skip 10",665,566,dark,.76f);
        textMid(r,m_fBody,m_profileAssets.catalog.counter(m_profiles.draft,m_profiles.field),1080,602,muted,.75f);
        textMid(r,m_fBody,std::to_string(m_profileAssets.catalog.items.size())+" catalog assets",665,602,muted,.75f);
        footer(88,"X","Save avatar",{.05f,.37f,.72f,1});footer(320,"B","Cancel",{.75f,.08f,.07f,1});footer(520,"Y","Randomize",{.84f,.66f,.04f,1});return;
    }
    if(welcome){
        const auto* profile=m_profiles.current();if(profile){drawCharacter(r,avatar360::forProfile(m_settings.profileAvatars,profile->id),400,575,310,0,true);textMid(r,m_fDashNav,"Welcome,",700,342,white,.8f);textMid(r,m_fDashNav,fit(m_fDashNav,profile->name,450,.9f),700,400,white,.9f);}
        footer(88,"A","Continue",green);return;
    }
    if(m_profiles.profiles.empty())centered(m_fBody,"No Switch profiles found. Choose Manage users to add one.",640,360,white);
    r.pushClipRect({0,140,1280,500});
    for(int i=0;i<int(m_profiles.profiles.size());++i){
        const auto box=avatar360::profileBox(i,m_profileScroll);if(box.x+box.w<0 || box.x>1280)continue;
        const auto& profile=m_profiles.profiles[i];const bool chosen=i==m_profiles.selected;const float x=box.x+box.w/2;
        // A broad soft floor shadow anchors each full-body character.
        for(int n=8;n>0;--n)r.drawRoundedRect({x-30-n*3.f,485-n*.7f,60+n*6.f,9+n*1.4f},{0,0,0,.012f},10);
        drawCharacter(r,avatar360::forProfile(m_settings.profileAvatars,profile.id),x,490,chosen?210:200,chosen?-.08f:.08f,false,false,chosen?1.f:.8f);
        centered(m_fLabel,fit(m_fLabel,profile.name,260,chosen?1.15f:.95f),x,532,chosen?dark:muted,chosen?1.15f:.95f);
        centered(m_fBody,profile.id==profile360Id(m_user)?"Signed in":"Offline profile",x,568,chosen?dark:muted,.76f);
        if(chosen)r.drawRect({x-55,597,110,3},green);
    }
    r.popClipRect();
    footer(88,"A","Select",green);footer(270,"B","Back",{.75f,.08f,.07f,1});footer(440,"X","Edit avatar",{.05f,.37f,.72f,1});footer(680,"Y","Manage users",{.84f,.66f,.04f,1});
    if(m_profiles.profiles.size()>4)textMid(r,m_fBody,std::to_string(m_profiles.selected+1)+" / "+std::to_string(m_profiles.profiles.size()),1120,666,dark,.8f);
}
