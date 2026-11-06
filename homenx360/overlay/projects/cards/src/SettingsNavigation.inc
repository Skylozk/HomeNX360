// Production settings controller and touch behavior, also exercised by host tests.
void CardsActivity::openNxePage(int page) {
    if(!settings360::enter(page,m_sheetSec,m_sheetRow,m_sheetHistory)) return;
    m_sheetOnRows=true;m_sheetScroll=0;m_sheetFocus=0;m_sheetDirection=1;
    m_sheetRowsIn=m_settings.motion?0.f:1.f;m_audio.play("tab_transition");
}
void CardsActivity::backNxeSettings() {
    if(settings360::back(m_sheetSec,m_sheetRow,m_sheetHistory)) {
        const int count=int(m_sheet[m_sheetSec].rows.size());
        m_sheetRow=std::clamp(m_sheetRow,0,std::max(0,count-1));
        m_sheetScroll=settings360::topRow(m_sheetRow,count)*settings360::pitch;
        m_sheetFocus=float(m_sheetRow);m_sheetDirection=-1;
        m_sheetRowsIn=m_settings.motion?0.f:1.f;m_audio.play("hide_modal");
    } else {
        m_sheetClosing=m_settings.motion;
        if(!m_sheetClosing) m_sheetOpen=false;
        m_audio.play("hide_modal");
    }
}
void CardsActivity::activateNxeSetting(int delta,bool stepsOnly) {
    if(m_sheetSec<0 || m_sheetSec>=int(m_sheet.size())) return;
    auto& rows=m_sheet[m_sheetSec].rows;
    if(rows.empty()) return;
    m_sheetRow=std::clamp(m_sheetRow,0,int(rows.size())-1);
    // Copy callbacks before invoking: a callback can replace the current page.
    const auto item=rows[m_sheetRow];
    if(item.kind==MenuItem::Action && !stepsOnly && item.run) {
        auto run=item.run;m_audio.play("activation");run();
    } else if(item.kind!=MenuItem::Info && item.change) {
        takeFacts(true);auto change=item.change;change(delta);m_audio.play("navigation");
    }
}
bool CardsActivity::touchNxeSettings(float x,float y) {
    if(m_sheetClosing) return true;
    if(y>=625 && y<680) {
        if(x>=90 && x<200) activateNxeSetting();
        else if(x>=200 && x<320) backNxeSettings();
        return true;
    }
    const float slide=24.f*m_sheetDirection*(1.f-nxe::ease(m_sheetRowsIn));
    auto& rows=m_sheet[m_sheetSec].rows;
    const int hit=settings360::hitRow(x,y,int(rows.size()),m_sheetScroll,slide);
    if(hit>=0) {
        const bool wasSelected=hit==m_sheetRow;m_sheetRow=hit;
        m_audio.play("navigation");
        // A touch that only focuses a value must never change it accidentally.
        if(rows[hit].kind==MenuItem::Action || wasSelected) activateNxeSetting();
        return true;
    }
    // The current-setting area offers explicit minus/plus touch controls.
    if(x>=665 && x<1050 && y>=225 && y<285 && !rows.empty()) {
        const auto kind=rows[std::clamp(m_sheetRow,0,int(rows.size())-1)].kind;
        if(kind==MenuItem::Slider || kind==MenuItem::Choice || kind==MenuItem::Toggle)
            activateNxeSetting(x<855?-1:1,true);
    }
    return true;
}

bool CardsActivity::updateSheet(float dt) {
    auto& in=app().input();using B=nxui::Button;
    if(m_sheetClosing) return true;
    if(m_sheet.empty() || m_sheetSec<0 || m_sheetSec>=int(m_sheet.size())) return false;
    if(in.isDown(B::B)) {backNxeSettings();return true;}
    if(in.isDown(B::Plus)) {
        m_sheetHistory.clear();backNxeSettings();return true;
    }
    const int count=int(m_sheet[m_sheetSec].rows.size());
    if(!count) return true;
    const B buttons[][3]={{B::DUp,B::LStickU,B::RStickU},{B::DDown,B::LStickD,B::RStickD},
        {B::DLeft,B::LStickL,B::RStickL},{B::DRight,B::LStickR,B::RStickR}};
    int direction=-1;bool held=false,pressed=false;
    for(int i=0;i<4;++i) for(const auto button:buttons[i]) {
        if(in.isDown(button)) {pressed=true;direction=i;}
        if(in.isHeld(button)) {held=true;if(direction<0) direction=i;}
    }
    if(pressed) m_sheetRepeat=0.32f;
    else if(held && (m_sheetRepeat-=dt)<=0) {pressed=true;m_sheetRepeat=0.12f;}
    if(pressed && direction<2) {
        const int next=std::clamp(m_sheetRow+(direction==0?-1:1),0,count-1);
        if(next!=m_sheetRow) {m_sheetRow=next;m_audio.play("navigation");}
    } else if(pressed && direction>=2) activateNxeSetting(direction==2?-1:1,true);
    if(in.isDown(B::A)) activateNxeSetting();
    return true;
}

