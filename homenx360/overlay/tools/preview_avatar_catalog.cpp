// CPU geometry, textures and lighting shared with the native catalog renderer.
#include "../projects/cards/src/AvatarCatalog.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <nxui/third_party/stb/stb_image.h>
#include <iostream>
using namespace cards::avatar360;
template<class T> void write(std::ofstream& out,const T& value){out.write(reinterpret_cast<const char*>(&value),sizeof(value));}
static std::vector<uint8_t> decode(const std::string& path){int w,h,n;auto* p=stbi_load(path.c_str(),&w,&h,&n,4);if(!p)return {};std::vector<uint8_t> b(p,p+w*h*4);stbi_image_free(p);return b;}
int main(int argc,char** argv){
    if(argc<3)return 2;CatalogIndex index;if(!index.load(argv[1]))return 3;const bool editor=argc>3 && std::string(argv[3])=="editor";std::vector<CatalogAvatar> people;std::vector<Look> looks;
    for(int person=0;person<(editor?1:4);++person){Look look;look.value[Character]=person%2;index.initialize(look);
        const char* names[]={person%2?"Ponytail":"Fauxhawk",person%2?"Green Full-zip Hoodie":"Long Sleeved Tee",person%2?"Dark Flares":"Khaki Chinos",person%2?"Scarlet Pumps":"High Top Sneakers"};
        for(int g=0;g<4;++g){const auto opts=index.options(g,look.colors[0]);const auto it=std::find_if(opts.begin(),opts.end(),[&](auto i){return i && i->name.find(names[g])!=std::string::npos;});if(it!=opts.end())look.items[g]=(*it)->key;}
        look.colors[1]=std::array<int,4>{7,17,2,10}[person];look.colors[2]=std::array<int,4>{1,9,6,14}[person];
        if(person>=2){auto opts=index.options(20,look.colors[0]);auto it=std::find_if(opts.begin(),opts.end(),[&](auto i){return i && i->name.find(person==2?"Ezio Outfit":"Old Ezio Embers Outfit")!=std::string::npos;});if(it!=opts.end())look.items[20]=(*it)->key;}
        if(argc>3 && !editor && person==0){const auto* i=index.find(argv[3]);if(!i)return 6;look.colors[0]=(i->bodies&1)?0:1;index.initialize(look);look.items[i->group]=i->key;}
        auto avatar=buildCatalogAvatar(index,look,decode);if(!avatar.error.empty()){std::cerr<<avatar.error<<"\n";return 4;}people.push_back(std::move(avatar));looks.push_back(look);
    }
    std::ofstream out(argv[2],std::ios::binary);uint32_t images=0,parts=0;for(const auto& p:people)images+=p.model.images.size();write(out,images);const auto countPos=out.tellp();write(out,parts);
    for(const auto& p:people)for(size_t i=0;i<p.model.images.size();++i){auto raw=p.pixels.find(i);int w=256,h=256,n=0;uint8_t* image=nullptr;if(raw==p.pixels.end())image=stbi_load_from_memory(p.model.images[i].data(),p.model.images[i].size(),&w,&h,&n,4);if(!image && raw==p.pixels.end())return 5;write(out,uint32_t(w));write(out,uint32_t(h));const auto* data=raw!=p.pixels.end()?raw->second.data():image;out.write(reinterpret_cast<const char*>(data),w*h*4);if(image)stbi_image_free(image);}
    uint32_t texture=0;
    for(int person=0;person<int(people.size());++person){const auto& model=people[person].model;const auto& look=looks[person];const auto world=model.pose(argc>4?std::stof(argv[4]):1.2f,false),skin=retarget(model,model,world);
        for(int pass=0;pass<2;++pass)for(const auto& part:model.parts){if(int(part.blend)!=pass)continue;std::vector<ProjectedVertex> projected;std::vector<std::array<float,8>> vertices;
            const auto tint=part.slot==0?rgb(skinPalette[look.colors[1]]):part.slot==2?rgb(colorPalette[look.colors[2]]):std::array<float,3>{1,1,1};
            for(size_t i=0;i<part.vertices.size();++i){const auto& v=part.vertices[i];const auto p=projectVertex(v,skin,editor?330.f:190.f+335*person,editor?575.f:490.f,editor?310.f:230.f,editor?.3f:-.08f,1,1);projected.push_back(p);const float shade=catalogShade(v,part.normals[i],skin,editor?.3f:-.08f);const uint32_t c=part.colors[i];vertices.push_back({p.x,p.y,p.z,p.u,p.v,tint[0]*shade*(c&255)/255.f,tint[1]*shade*(c>>8&255)/255.f,tint[2]*shade*(c>>16&255)/255.f});}
            const auto indices=visibleIndices(part,projected);if(indices.empty())continue;write(out,uint32_t(texture+part.texture));write(out,part.blend);write(out,uint32_t(vertices.size()));write(out,uint32_t(indices.size()));for(int i=0;i<3;++i)write(out,-1.f);out.write(reinterpret_cast<const char*>(vertices.data()),vertices.size()*32);out.write(reinterpret_cast<const char*>(indices.data()),indices.size()*4);++parts;
        }texture+=model.images.size();
    }
    out.seekp(countPos);write(out,parts);return !out;
}
