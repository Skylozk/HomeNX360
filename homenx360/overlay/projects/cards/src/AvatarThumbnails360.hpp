#pragma once
#include <nxui/core/Texture.hpp>
#include <nxui/core/GpuDevice.hpp>
#include <nxui/third_party/stb/stb_image.h>
#include <future>
#include <deque>
#include <map>
#include <memory>
namespace cards::avatar360 {
// Only the eight visible item pictures are requested. Avatar geometry stays in
// its existing five-entry cache; item previews are small pre-rendered PNGs.
struct ThumbnailCache {
    struct Image {std::string key;std::vector<uint8_t> rgba;};
    struct Entry {std::unique_ptr<nxui::Texture> texture;uint64_t used=0;bool failed=false;};
    std::map<std::string,Entry> entries;std::deque<std::string> pending;
    std::future<Image> job;std::string loading;uint64_t frame=0;
    const nxui::Texture* request(const std::string& key){
        if(key.empty())return nullptr;
        if(auto it=entries.find(key);it!=entries.end()){it->second.used=frame;return it->second.texture.get();}
        if(key!=loading && std::find(pending.begin(),pending.end(),key)==pending.end()){pending.push_back(key);while(pending.size()>16)pending.pop_front();}return nullptr;
    }
    void pump(nxui::GpuDevice& gpu,nxui::Renderer& renderer,const std::string& catalog){
        ++frame;
        if(job.valid() && job.wait_for(std::chrono::seconds(0))==std::future_status::ready){
            Image image;try{image=job.get();}catch(...){image.key=loading;}Entry e;e.used=frame;
            if(!image.rgba.empty()){e.texture=std::make_unique<nxui::Texture>();if(!e.texture->loadFromPixels(gpu,renderer,image.rgba.data(),144,144))e.texture.reset();}
            e.failed=!e.texture;entries[image.key]=std::move(e);loading.clear();
        }
        while(entries.size()>12){auto oldest=entries.end();for(auto it=entries.begin();it!=entries.end();++it)if(it->second.used+2<frame && (oldest==entries.end() || it->second.used<oldest->second.used))oldest=it;if(oldest==entries.end())break;gpu.waitIdle();entries.erase(oldest);}
        if(!job.valid() && !pending.empty()){
            loading=pending.back();pending.pop_back();const auto key=loading,path=catalog+"/"+key;
            job=std::async(std::launch::async,[key,path](){Image image;image.key=key;int w=0,h=0,n=0;
                if(!stbi_info(path.c_str(),&w,&h,&n) || w!=144 || h!=144)return image;
                if(auto* pixels=stbi_load(path.c_str(),&w,&h,&n,4)){image.rgba.assign(pixels,pixels+144*144*4);stbi_image_free(pixels);}return image;
            });
        }
    }
    void clear(){if(job.valid())job.wait();job={};entries.clear();pending.clear();loading.clear();}
};
}
