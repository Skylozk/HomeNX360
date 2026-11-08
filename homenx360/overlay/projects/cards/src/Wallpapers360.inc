// Bounded, offline NXE wallpaper previews. The custom-picture picker stays separate.
void CardsActivity::openNxeWallpapers(){
    if(m_pickJob.valid())m_pickJob.wait();
    m_pickWall=4;m_pickCover=false;m_pickQuery.clear();
    m_pickChoices.clear();m_pickTex.clear();m_pickSel=0;
    const auto current=m_settings.wallpaper;
    const auto assets=std::string(kAssets);
    m_pickJob=std::async(std::launch::async,[current,assets](){
        offScreenCore();
        std::vector<TitleChoice> choices;
        choices.emplace_back().own=true;
        choices.front().before=current.empty();
        for(const auto& entry:wallpaper360::entries){
            const auto saved=wallpaper360::selection(entry);
            auto pixels=readPicture(wallpaper360::resolve(saved,assets),320);
            if(pixels.rgba.empty())continue;
            TitleChoice choice;
            choice.file=saved;choice.label=entry.name;choice.before=saved==current;
            choice.w=pixels.w;choice.h=pixels.h;choice.rgba=std::move(pixels.rgba);
            choices.push_back(std::move(choice));
        }
        threadDone();return choices;
    });
    if(!m_pickOpen){m_pickIn=m_settings.motion?0.f:1.f;m_audio.play("show_modal");}
    m_pickOpen=true;
}
