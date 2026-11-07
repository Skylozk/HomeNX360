#pragma once
#include "Avatar360.hpp"
#include "AvatarModel.hpp"
#include <nxui/core/Renderer.hpp>
#include <nxui/core/Texture.hpp>
#include <memory>

namespace cards::avatar360 {
struct Assets {
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
    void releaseTextures(){for(auto& set:textures)set.clear();}
    bool available()const{if(!ready)return false;for(size_t i=0;i<models.size();++i){if(!models[i].valid() || textures[i].size()!=models[i].images.size())return false;for(const auto& t:textures[i])if(!t)return false;}return true;}
};
inline bool drawCharacter(Assets& assets,nxui::Renderer& renderer,Look look,float x,float base,float scale,float yaw,float time,float alpha=1,bool wave=false,bool portrait=false){
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
