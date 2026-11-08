#pragma once
#include "Avatar360.hpp"
#include "AvatarCatalog.hpp"
#include <nxui/third_party/stb/stb_image.h>
#include <future>
#include <deque>
#include <nxui/core/Renderer.hpp>
#include <nxui/core/Texture.hpp>
#include <memory>

namespace cards::avatar360 {
struct AssetData {std::array<Model,6> models;CatalogIndex catalog;};
struct Assets {
    CatalogIndex catalog;
    struct Entry {CatalogAvatar avatar;std::vector<std::unique_ptr<nxui::Texture>> textures;uint64_t used=0;};
    std::map<std::string,Entry> cache;
    std::deque<Look> pending;
    std::future<CatalogAvatar> job;std::string jobKey;uint64_t frame=0;
    static std::vector<uint8_t> decodeImage(const std::string& path){
        int w=0,h=0,n=0;if(!stbi_info(path.c_str(),&w,&h,&n)||w!=256||h!=256)return {};
        auto* p=stbi_load(path.c_str(),&w,&h,&n,4);if(!p)return {};
        std::vector<uint8_t> pixels(p,p+256*256*4);stbi_image_free(p);return pixels;
    }
    Entry* request(const Look& look){
        const auto key=lookKey(look);auto it=cache.find(key);
        if(it!=cache.end()){it->second.used=frame;return &it->second;}
        if(jobKey!=key && std::none_of(pending.begin(),pending.end(),[&](const auto& l){return lookKey(l)==key;})){pending.push_back(look);while(pending.size()>5)pending.pop_front();}return nullptr;
    }
    void pump(nxui::GpuDevice& gpu,nxui::Renderer& renderer){
        ++frame;
        if(job.valid() && job.wait_for(std::chrono::seconds(0))==std::future_status::ready){
            Entry e;try{e.avatar=job.get();}catch(...){e.avatar.error="Avatar conversion could not be loaded.";}e.used=frame;
            e.textures.resize(e.avatar.model.images.size());cache[jobKey]=std::move(e);jobKey.clear();
        }
        while(cache.size()>5){auto oldest=cache.end();for(auto it=cache.begin();it!=cache.end();++it)if(it->second.used+2<frame && (oldest==cache.end() || it->second.used<oldest->second.used))oldest=it;if(oldest==cache.end())break;cache.erase(oldest);}
        if(!job.valid() && !pending.empty() && catalog.ready){const Look next=pending.back();pending.pop_back();jobKey=lookKey(next);
            job=std::async(std::launch::async,[this,next](){return buildCatalogAvatar(catalog,next,decodeImage);});}
        for(auto& [key,e]:cache)if(e.used+2>=frame && e.avatar.error.empty())for(size_t i=0;i<e.textures.size();++i)if(!e.textures[i]){
            auto texture=std::make_unique<nxui::Texture>();const auto raw=e.avatar.pixels.find(i);
            const bool ok=raw==e.avatar.pixels.end()?texture->loadFromMemory(gpu,renderer,e.avatar.model.images[i].data(),e.avatar.model.images[i].size()):texture->loadFromPixels(gpu,renderer,raw->second.data(),256,256);
            if(ok)e.textures[i]=std::move(texture);else e.avatar.error="Avatar texture upload failed.";return;
        }
    }
    std::array<Model,6> models;
    std::array<std::vector<std::unique_ptr<nxui::Texture>>,6> textures;
    bool ready=false;
    // Upload at most one texture per frame. Until complete, the picker shows
    // a loading message instead of an incomplete, untextured character.
    bool upload(nxui::GpuDevice& gpu,nxui::Renderer& renderer){
        if(!ready)return false;
        for(size_t i=0;i<models.size();++i){
            auto& model=models[i];if(!model.valid())return false;
            auto& set=textures[i];if(set.empty())set.resize(model.images.size());
            for(size_t j=0;j<set.size();++j)if(!set[j]){
                auto tex=std::make_unique<nxui::Texture>();
                if(!tex->loadFromMemory(gpu,renderer,model.images[j].data(),model.images[j].size()))return false;
                set[j]=std::move(tex);return false;
            }
        }
        return true;
    }
    void releaseTextures(){for(auto& set:textures)set.clear();cache.clear();pending.clear();}
    bool available()const{if(!ready)return false;for(size_t i=0;i<models.size();++i){if(!models[i].valid() || textures[i].size()!=models[i].images.size())return false;for(const auto& t:textures[i])if(!t)return false;}return true;}
};
inline bool drawCatalogCharacter(Assets& assets,nxui::Renderer& renderer,const Look& look,float x,float base,float scale,float yaw,float time,float alpha,bool wave,bool portrait){
    auto* e=assets.request(look);if(!e || !e->avatar.error.empty() || e->textures.empty() || std::any_of(e->textures.begin(),e->textures.end(),[](const auto& t){return !t;}))return false;
    const auto& model=e->avatar.model;const auto world=model.pose(time,wave),skin=retarget(model,model,world);if(skin.empty())return false;
    renderer.beginAvatarPass();bool ok=true;
    for(int transparent=0;transparent<2;++transparent)for(const auto& part:model.parts){
        if(int(part.blend)!=transparent || (portrait && part.slot!=1 && part.slot!=2) || (wave && part.slot==4))continue;
        const auto tint=part.slot==0?rgb(skinPalette[look.colors[1]]):part.slot==2?rgb(colorPalette[look.colors[2]]):std::array<float,3>{1,1,1};
        std::vector<ProjectedVertex> projected;std::vector<nxui::Vertex2D> vertices;projected.reserve(part.vertices.size());vertices.reserve(part.vertices.size());
        for(size_t i=0;i<part.vertices.size();++i){const auto& v=part.vertices[i];const auto p=projectVertex(v,skin,x,base,scale,yaw,.9f+.05f*look.colors[8],.88f+.06f*look.colors[9],portrait);
            const float shade=i<part.normals.size()?catalogShade(v,part.normals[i],skin,yaw):1.f;const uint32_t color=i<part.colors.size()?part.colors[i]:0xffffffff;
            projected.push_back(p);vertices.push_back({p.x,p.y,p.u,p.v,tint[0]*shade*(color&255)/255.f,tint[1]*shade*(color>>8&255)/255.f,tint[2]*shade*(color>>16&255)/255.f,alpha,p.z,0,0,0,-1.f,0});}
        const auto indices=visibleIndices(part,projected);if(!indices.empty() && !renderer.drawAvatarMesh(e->textures[part.texture].get(),vertices,indices,part.blend))ok=false;
    }
    renderer.endAvatarPass();return ok;
}
inline bool drawCharacter(Assets& assets,nxui::Renderer& renderer,Look look,float x,float base,float scale,float yaw,float time,float alpha=1,bool wave=false,bool portrait=false){
    if(look.expanded)return drawCatalogCharacter(assets,renderer,look,x,base,scale,yaw,time,alpha,wave,portrait);
    if(!assets.available())return false;look.normalize();const auto& values=look.value;
    const int main=values[Character];const auto& body=assets.models[main];
    const auto world=body.pose(time,wave);
    renderer.beginAvatarPass();bool ok=true;
    for(int transparent=0;transparent<2;++transparent)for(int source=0;source<6;++source){
        const auto& model=assets.models[source];const auto skin=retarget(model,body,world);if(skin.empty()){ok=false;continue;}
        for(const auto& part:model.parts){
            if(int(part.blend)!=transparent)continue;
            if(!partEnabled(look,part.slot,source) || (portrait && part.slot!=1 && part.slot!=2 && part.slot!=6))continue;
            std::vector<ProjectedVertex> projected;projected.reserve(part.vertices.size());
            std::vector<nxui::Vertex2D> vertices;vertices.reserve(part.vertices.size());
            for(const auto& v:part.vertices){
                const auto p=projectVertex(v,skin,x,base,scale,yaw,.9f+.05f*values[Height],.88f+.06f*values[Build],portrait);
                const bool colorHair=part.slot==2 && values[HairColour]!=0;
                const auto tint=colorHair?hairTints[values[HairColour]]:std::array<float,3>{1,1,1};
                projected.push_back(p);vertices.push_back({p.x,p.y,p.u,p.v,tint[0],tint[1],tint[2],alpha,p.z,0,0,0,colorHair?-2.f:-1.f,0});
            }
            const auto indices=visibleIndices(part,projected);
            if(!indices.empty() && !renderer.drawAvatarMesh(assets.textures[source][part.texture].get(),vertices,indices,part.blend))ok=false;
        }
    }
    renderer.endAvatarPass();return ok;
}
} // namespace cards::avatar360
