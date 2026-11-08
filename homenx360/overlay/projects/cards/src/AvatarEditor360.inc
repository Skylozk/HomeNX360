// NXE-style category tabs, eight actual item pictures and one live avatar.
void CardsActivity::randomizeProfileAvatar(){
    auto& look=m_profiles.draft;uint32_t seed=uint32_t(armGetSystemTick());
    auto next=[&](){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;};if(!seed)seed=0x360360;
    for(int i=0;i<12;++i)look.colors[i]=next()%avatar360::colorCount(i);
    auto& catalog=m_profileAssets.catalog;catalog.initialize(look);
    for(int group=0;group<21;++group){auto options=catalog.options(group,look.colors[0]);if(!options.empty()){const auto* item=options[next()%options.size()];look.items[group]=item?item->key:"";}}
    m_avatarEditor.synchronize(catalog,look);
}
void CardsActivity::updateAvatarEditor(float dt){
    using B=nxui::Button;auto& input=app().input();auto& editor=m_avatarEditor;
    auto& index=m_profileAssets.catalog;auto& draft=m_profiles.draft;
    m_avatarThumbs.pump(app().gpu(),app().renderer(),index.path);
    bool changed=false;
    if(input.isDown(B::L)){editor.category(index,draft,-1);changed=true;}
    if(input.isDown(B::R)){editor.category(index,draft,1);changed=true;}
    if(input.isDown(B::Y)){editor.optionOrTab(index,draft,1);changed=true;}
    if(input.isDown(B::Minus)){editor.optionOrTab(index,draft,-1);changed=true;}
    if(input.isDown(B::ZL)){editor.turn(index,draft,-1);changed=true;}
    if(input.isDown(B::ZR)){editor.turn(index,draft,1);changed=true;}
    const bool up=input.isDown(B::DUp)||input.isDown(B::LStickU),down=input.isDown(B::DDown)||input.isDown(B::LStickD);
    const bool left=input.isDown(B::DLeft)||input.isDown(B::LStickL),right=input.isDown(B::DRight)||input.isDown(B::LStickR);
    int vertical=down-up,horizontal=right-left;
    const bool held=input.isHeld(B::DUp)||input.isHeld(B::DDown)||input.isHeld(B::DLeft)||input.isHeld(B::DRight)||input.isHeld(B::LStickU)||input.isHeld(B::LStickD)||input.isHeld(B::LStickL)||input.isHeld(B::LStickR);
    if(up||down||left||right)m_profileRepeat=.35f;
    else if(held && (m_profileRepeat-=dt)<=0){m_profileRepeat=.13f;vertical=(input.isHeld(B::DDown)||input.isHeld(B::LStickD))-(input.isHeld(B::DUp)||input.isHeld(B::LStickU));horizontal=(input.isHeld(B::DRight)||input.isHeld(B::LStickR))-(input.isHeld(B::DLeft)||input.isHeld(B::LStickL));}
    if(horizontal || vertical){editor.move(index,draft,horizontal,vertical);changed=true;}
    if(input.isHeld(B::RStickL))m_profiles.yaw-=dt*1.8f;
    if(input.isHeld(B::RStickR))m_profiles.yaw+=dt*1.8f;
    if(input.isDown(B::RStick))editor.zoom=!editor.zoom;
    if(input.isDown(B::A)){editor.use(index,draft);m_audio.play("toggle_on");}
    if(input.isDown(B::Plus)){randomizeProfileAvatar();changed=true;}
    if(input.isDown(B::X)){editor.use(index,draft);saveProfileAvatar();return;}
    if(input.touchUp()){
        const float x=input.touchX(),y=input.touchY();
        if(y>=643){
            if(x<180)editor.use(index,draft);
            else if(x<340)m_profiles.back();
            else if(x<550){editor.use(index,draft);saveProfileAvatar();}
            else if(x<760)editor.optionOrTab(index,draft,1);
            else if(x<980)randomizeProfileAvatar();
            else editor.zoom=!editor.zoom;
        }else if(y>=54 && y<106){
            for(int tab=0;tab<int(avatar360::editor::tabs.size());++tab)if(avatar360::editor::tabBox(tab).contains(x,y)){editor.category(index,draft,tab-editor.tab);changed=true;break;}
        }else if(y>=120 && y<170){editor.optionOrTab(index,draft,x<640?-1:1);changed=true;}
        else if(x>390 && x<890 && std::abs(input.touchDeltaX())>12)m_profiles.yaw+=input.touchDeltaX()*.009f;
        else {
            const int total=avatar360::editor::count(index,draft,editor.field());
            for(int tile=0;tile<8;++tile)if(avatar360::editor::tileBox(tile).contains(x,y)){
                const int at=editor.page()*8+tile;if(at>=total)break;
                if(at==editor.focus)editor.use(index,draft);else editor.browse(index,draft,at);changed=true;break;
            }
            if(y>=340 && y<470 && (x<160 || x>1120)){editor.turn(index,draft,x<160?-1:1);changed=true;}
        }
    }
    m_profiles.field=editor.field();if(changed)m_audio.play("navigation");
}
void CardsActivity::drawAvatarEditor(nxui::Renderer& r){
    using namespace avatar360;auto& browser=m_avatarEditor;auto& index=m_profileAssets.catalog;
    const auto& draft=m_profiles.draft;const int field=browser.field(),total=editor::count(index,draft,field),begin=browser.page()*8;
    const nxui::Color white{1,1,1,1},dark{.14f,.20f,.10f,1},muted{.30f,.36f,.25f,1},green{.39f,.69f,.035f,1};
    auto centered=[&](nxui::Font& font,const std::string& text,float x,float y,nxui::Color color,float scale=1.f){textMid(r,font,text,x-measured(font,text).x*scale/2,y,color,scale);};
    r.drawGradientRect({0,0,1280,470},{.21f,.38f,.015f,1},{.69f,.80f,.38f,1});
    r.drawGradientRect({0,470,1280,250},{.69f,.80f,.38f,1},{.91f,.93f,.86f,1});
    for(int stripe=0;stripe<24;++stripe)r.drawLine({float(stripe*61-350),0},{float(stripe*61+100),470},{1,1,1,.025f},2);
    textMid(r,m_fDashNav,"avatar editor",65,28,white,.65f);
    if(const auto* profile=m_profiles.current())textMid(r,m_fBody,fit(m_fBody,profile->name,380,.8f),1195-measured(m_fBody,fit(m_fBody,profile->name,380,.8f)).x*.8f,28,white,.8f);
    for(int tab=0;tab<int(editor::tabs.size());++tab){const auto b=editor::tabBox(tab);const bool current=tab==browser.tab;
        if(current)r.drawGradientRect({b.x,b.y,b.w,b.h},{.58f,.85f,.12f,1},{.35f,.60f,.025f,1});
        else r.drawRect({b.x,b.y,b.w,b.h},{.10f,.22f,.015f,.28f});
        centered(m_fBody,editor::tabs[tab].name,b.x+b.w/2,b.y+b.h/2,white,.9f);
        if(current)r.drawRect({b.x,b.y+b.h-3,b.w,3},{.91f,1,.61f,1});
    }
    centered(m_fGuide,catalogFieldNames[field],640,143,white,.95f);
    textMid(r,m_fBody,"<",400,143,white);textMid(r,m_fBody,">",864,143,white);
    textMid(r,m_fBody,"L / R  Category",78,143,white,.7f);
    textMid(r,m_fBody,editor::tabs[browser.tab].count>1?"Y / -  Options":"Y / -  Category",1020,143,white,.7f);
    const int equipped=editor::selected(index,draft,field);bool missing=false;
    for(int tile=0;tile<8 && begin+tile<total;++tile){const int at=begin+tile;const auto b=editor::tileBox(tile);const bool focused=at==browser.focus;
        r.drawRect({b.x-2,b.y-2,b.w+4,b.h+4},focused?green:nxui::Color(.22f,.31f,.17f,.5f));
        r.drawGradientRect({b.x,b.y,b.w,b.h},focused?nxui::Color(.84f,.95f,.62f,1):nxui::Color(.76f,.82f,.69f,1),focused?nxui::Color(.53f,.76f,.23f,1):nxui::Color(.52f,.61f,.44f,1));
        const int color=fieldColor(field);const auto picture=editor::thumbnail(index,draft,field,at);
        if(!picture.empty()){
            if(const auto* texture=m_avatarThumbs.request(picture))r.drawTexture(texture,{b.x+34,b.y+2,76,76});
            else {centered(m_fBody,"...",b.x+b.w/2,b.y+35,muted);auto it=m_avatarThumbs.entries.find(picture);missing|=it!=m_avatarThumbs.entries.end() && it->second.failed;}
        }else if(color>=0 && color!=8 && color!=9){const auto c=rgb(color==1?skinPalette[at]:colorPalette[at]);
            r.drawRoundedRect({b.x+46,b.y+10,52,52},{c[0],c[1],c[2],1},26);
            r.drawRoundedRectOutline({b.x+46,b.y+10,52,52},white.withAlpha(.5f),26,1);
        }else {
            const float w=color==9?20+at*5.f:30,h=color==8?25+at*7.f:46;
            r.drawRoundedRect({b.x+b.w/2-w/2,b.y+65-h,w,h},dark.withAlpha(.72f),10);
            r.drawCircle({b.x+b.w/2,b.y+54-h},9,dark.withAlpha(.72f));
        }
        centered(m_fBody,fit(m_fBody,editor::name(index,draft,field,at),b.w-12,.57f),b.x+b.w/2,b.y+85,dark,.57f);
        if(at==equipped)r.drawCircle({b.x+b.w-9,b.y+10},4,{.15f,.38f,.015f,1});
        if(focused)r.drawRoundedRectOutline({b.x-3,b.y-3,b.w+6,b.h+6},white,0,2);
    }
    centered(m_fDashNav,"<",113,405,white,.9f);centered(m_fDashNav,">",1172,405,white,.9f);
    centered(m_fBody,"Page "+std::to_string(browser.page()+1)+" of "+std::to_string(browser.pages(index,draft)),272,634,dark,.72f);
    centered(m_fBody,"ZL / ZR  More",1008,634,dark,.72f);
    auto* candidate=m_profileAssets.request(browser.preview);const bool ready=Assets::drawable(candidate);
    if(ready){
        if(!m_editorShown || *m_editorShown!=browser.preview){
            const auto& model=candidate->avatar.model;const auto skin=retarget(model,model,model.pose(1.2f,false));m_editorTop=1.f;
            for(const auto& part:model.parts)if(part.slot!=4)for(const auto& vertex:part.vertices){const auto p=skinVertex(vertex,skin);if(p.y<4.f)m_editorTop=std::max(m_editorTop,p.y*(.9f+.05f*browser.preview.colors[8]));}
        }
        m_editorShown=browser.preview;
    }
    r.pushClipRect({390,170,500,447});
    for(int n=7;n>0;--n)r.drawRoundedRect({584-n*3.f,596-n*.7f,112+n*6.f,8+n*1.4f},{0,0,0,.012f},10);
    const float zoom=std::min(620.f,414.f/std::max(.4f,m_editorTop-.65f));
    if(m_editorShown)drawCharacter(r,*m_editorShown,640,browser.zoom?190.f+m_editorTop*zoom:604.f,browser.zoom?zoom:std::min(340.f,414.f/m_editorTop),m_profiles.yaw);
    r.popClipRect();
    const auto name=editor::name(index,draft,field,browser.focus);
    centered(m_fBody,fit(m_fBody,name,480,.8f),640,632,dark,.8f);
    if(missing)centered(m_fBody,"Install the updated catalog for item pictures.",640,181,white,.62f);
    else if(!ready)centered(m_fBody,"Updating preview...",640,181,white,.65f);
    auto button=[&](float x,const char* letter,const char* label,nxui::Color color){r.drawCircle({x,679},10,color);centered(m_fBadge,letter,x,679,white,.75f);textMid(r,m_fBody,label,x+16,679,dark,.78f);};
    button(67,"A","Use",green);button(223,"B","Cancel",{.75f,.08f,.07f,1});button(383,"X","Save avatar",{.05f,.37f,.72f,1});button(586,"Y",editor::tabs[browser.tab].count>1?"Options":"Next tab",{.84f,.66f,.04f,1});
    button(795,"+","Randomize",{.25f,.33f,.22f,1});button(1012,"R","Zoom / rotate",{.25f,.33f,.22f,1});
}
