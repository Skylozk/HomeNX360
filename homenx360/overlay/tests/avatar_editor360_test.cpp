#include "../projects/cards/src/AvatarEditor360.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <nxui/third_party/stb/stb_image.h>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <set>
using namespace cards::avatar360;
int main(int argc,char**argv){
    assert(argc==2);CatalogIndex index;assert(index.load(argv[1]));std::set<std::string> pictures;
    for(int body=0;body<2;++body){
        Look draft;draft.value[Character]=body;index.initialize(draft);editor::Browser browser;browser.begin(index,draft);
        assert(browser.field()==CHat && browser.tab==0);std::array<bool,CFieldCount> reached{};
        browser.optionOrTab(index,draft,1);assert(browser.tab==1 && browser.field()==CHair);
        browser.optionOrTab(index,draft,1);assert(browser.field()==CHairColor);
        browser.category(index,draft,-1);assert(browser.field()==CHat);
        for(int tab=0;tab<int(editor::tabs.size());++tab){
            if(tab)browser.category(index,draft,1);assert(browser.tab==tab);
            for(int option=0;option<editor::tabs[tab].count;++option){
                if(option)browser.setting(index,draft,1);const int field=browser.field();reached[field]=true;
                const int total=editor::count(index,draft,field);assert(total>0);
                const auto before=draft;browser.browse(index,draft,total-1);
                assert(draft==before && browser.focus==total-1 && editor::selected(index,browser.preview,field)==total-1);
                browser.turn(index,draft,99999);assert(browser.focus==total-1);
                browser.turn(index,draft,-99999);assert(browser.focus==0);
                browser.move(index,draft,-1,-1);assert(browser.focus==0);
                browser.browse(index,draft,total-1);const auto candidate=browser.preview;
                Look used=draft;browser.use(index,used);assert(used==candidate && draft==before);
                for(int at=0;at<total;++at){
                    const auto picture=editor::thumbnail(index,draft,field,at);
                    assert(!editor::name(index,draft,field,at).empty());
                    if(!picture.empty())pictures.insert(picture);
                    browser.browse(index,draft,at);assert(browser.page()>=0 && browser.page()<browser.pages(index,draft));
                    const int tile=at%8;const auto box=editor::tileBox(tile);assert(box.y>=200 && box.y+box.h<=620 && box.contains(box.x+1,box.y+1));
                }
                browser.synchronize(index,draft);assert(browser.preview==draft && browser.focus==editor::selected(index,draft,field));
            }
        }
        for(bool field:reached)assert(field);
        browser.category(index,draft,1);assert(browser.tab==0 && browser.field()==CHat);
        browser.category(index,draft,-1);assert(browser.tab==6);
        // Browsing is a temporary try-on. Canceling the session preserves the save.
        const std::string id="00000000000000010000000000000002";Wardrobe wardrobe{{id,draft}};Session session;session.begin(false);session.loaded({{id,"Player"}},id);session.edit(wardrobe);index.initialize(session.draft);
        browser.begin(index,session.draft);browser.browse(index,session.draft,editor::count(index,draft,CHat)-1);browser.use(index,session.draft);session.back();assert(wardrobe.at(id)==draft);
        session.edit(wardrobe);browser.begin(index,session.draft);browser.browse(index,session.draft,editor::count(index,draft,CHat)-1);browser.use(index,session.draft);const auto saved=session.draft;assert(session.save(wardrobe) && wardrobe.at(id)==saved);
    }
    assert(pictures.size()==2420);
    size_t bytes=0;
    for(const auto& name:pictures){const auto path=std::filesystem::path(argv[1])/name;int width=0,height=0,channels=0;auto* png=stbi_load(path.string().c_str(),&width,&height,&channels,4);assert(png && width==144 && height==144);size_t visible=0;for(int i=3;i<144*144*4;i+=4)visible+=png[i]>0;assert(visible>=10);stbi_image_free(png);bytes+=std::filesystem::file_size(path);}
    std::cout<<"All 33 editor fields, category/option navigation, item paging, try-on/use/save/cancel and "<<pictures.size()<<" real model thumbnails passed ("<<bytes<<" PNG bytes).\n";
}
