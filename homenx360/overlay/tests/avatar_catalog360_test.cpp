#include "../projects/cards/src/AvatarCatalog.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <nxui/third_party/stb/stb_image.h>
#include <cassert>
#include <filesystem>
#include <iostream>
using namespace cards::avatar360;
static std::vector<uint8_t> decode(const std::string& path){int w=0,h=0,n=0;auto* p=stbi_load(path.c_str(),&w,&h,&n,4);if(!p)return {};assert(w==256 && h==256);std::vector<uint8_t>b(p,p+w*h*4);stbi_image_free(p);return b;}
int main(int argc,char**argv){
    assert(argc==2);CatalogIndex index;assert(index.load(argv[1]));assert(index.items.size()>1800);
    int banks=0;for(const auto& f:std::filesystem::directory_iterator(argv[1]))if(f.path().extension()==".avb"){Model m;assert(loadModel(f.path(),m)&&m.wearable());for(const auto& p:m.parts)assert(p.normals.size()==p.vertices.size() && p.colors.size()==p.vertices.size());++banks;}
    size_t largest=0,worstFour=0;int combinations=0;
    for(int body=0;body<2;++body){
        Look base;base.value[Character]=body;index.initialize(base);assert(base.expanded && base.colors[0]==body);
        auto neutral=buildCatalogAvatar(index,base,decode);assert(neutral.model.valid() && neutral.error.empty());
        Look worst=base;std::array<size_t,21> maximum{};
        for(int group=0;group<21;++group){const auto options=index.options(group,body);
            for(auto* item:options){Look look=base;look.items[group]=item?item->key:"";
                auto avatar=buildCatalogAvatar(index,look,decode);
                if(!avatar.error.empty())std::cerr<<(item?item->name:"None")<<": "<<avatar.error<<"\n";
                assert(avatar.error.empty() && avatar.model.valid());++combinations;
                size_t stream=0;for(const auto& p:avatar.model.parts)stream+=p.vertices.size()*56+p.indices.size()*4+512;largest=std::max(largest,stream);if(stream>maximum[group]){maximum[group]=stream;worst.items[group]=item?item->key:"";}
                for(bool wave:{false,true})for(float time:{0.f,2.7f,17.f}){
                    const auto skin=retarget(avatar.model,avatar.model,avatar.model.pose(time,wave));assert(skin.size()==avatar.model.bones.size());
                    for(const auto& part:avatar.model.parts)for(const auto& v:part.vertices){const auto p=skinVertex(v,skin);assert(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z));// Prop explosion fragments legitimately travel far outside the camera.
                        if(group!=19)assert(std::abs(p.x)<5 && std::abs(p.y)<5 && std::abs(p.z)<5);}
                }
                if(group>=16 && group<=18 && item){
                    bool changed=false;for(size_t p=1;p<3;++p)for(size_t v=0;v<neutral.model.parts[p].vertices.size();++v){const auto a=neutral.model.parts[p].vertices[v].position,b=avatar.model.parts[p].vertices[v].position;if(std::abs(a.x-b.x)+std::abs(a.y-b.y)+std::abs(a.z-b.z)>1e-5f)changed=true;}
                    // These three original choices are the authored neutral
                    // chin/nose/ears. Every other choice must change geometry.
                    assert(changed || item->key=="801" || item->key=="807" || item->key=="823");
                }
            }
        }
        for(bool outfit:{false,true}){auto look=worst;if(!outfit)look.items[20].clear();auto avatar=buildCatalogAvatar(index,look,decode);assert(avatar.error.empty());size_t stream=0;for(const auto& p:avatar.model.parts)stream+=p.vertices.size()*56+p.indices.size()*4+512;worstFour=std::max(worstFour,stream*4);}
        auto painted=base;auto paint=std::find_if(index.items.begin(),index.items.end(),[](const auto& i){return i.name=="Camo Face Paint";});assert(paint!=index.items.end());painted.items[14]=paint->key;
        const auto face=buildCatalogAvatar(index,painted,decode).pixels;painted.colors[10]=(painted.colors[10]+8)%32;const auto recolored=buildCatalogAvatar(index,painted,decode).pixels;assert(face!=recolored);
        Look edit=base;for(int field=0;field<CFieldCount;++field){const auto old=edit;index.cycle(edit,field,1);index.cycle(edit,field,-1);normalizeCatalog(edit);assert(edit.colors[0]>=0 && edit.colors[0]<2);}
    }
    // Bounded decompression rejects size bombs, corrupt zlib and trailing data.
    Model untouched=index.bases[0];const auto bones=untouched.bones;
    std::vector<uint8_t> bad(20,0);std::memcpy(bad.data(),"NXAZIP1\0",8);uint32_t bomb=0xffffffff;std::memcpy(bad.data()+8,&bomb,4);assert(!decodeModel(bad,untouched)&&untouched.bones==bones);
    std::ifstream f(std::string(argv[1])+"/base-1.avb",std::ios::binary);std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)),{});bytes.push_back(0);assert(!decodeModel(bytes,untouched));
    assert(worstFour<=8u*1024u*1024u);
    std::cout<<"Four-avatar catalog stream upper bound "<<worstFour<<" / 8388608 bytes.\n";
    std::cout<<banks<<" banks, "<<combinations<<" equipped appearances; largest single-avatar indexed stream "<<largest<<" bytes.\n";
}
