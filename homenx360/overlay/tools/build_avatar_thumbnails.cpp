// Offline thumbnails from the same models, skinning, masks and lighting as HOME.
#include "../projects/cards/src/AvatarCatalog.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <nxui/third_party/stb/stb_image.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace cards::avatar360;
namespace {
constexpr int size=144;
struct Texture {int w=0,h=0;std::vector<uint8_t> pixels;};
std::vector<uint8_t> decode(const std::string& path){
    static std::map<std::string,std::vector<uint8_t>> masks;
    if(auto it=masks.find(path);it!=masks.end())return it->second;
    int w,h,n;auto* p=stbi_load(path.c_str(),&w,&h,&n,4);if(!p)return {};
    std::vector<uint8_t> b(p,p+w*h*4);stbi_image_free(p);masks[path]=b;return b;
}
void big(std::vector<uint8_t>& b,uint32_t n){for(int s:{24,16,8,0})b.push_back(n>>s);}
void chunk(std::vector<uint8_t>& out,const char* type,const std::vector<uint8_t>& data){
    big(out,data.size());const auto start=out.size();out.insert(out.end(),type,type+4);out.insert(out.end(),data.begin(),data.end());big(out,crc32(0,out.data()+start,4+data.size()));
}
void png(const std::filesystem::path& path,const std::vector<uint8_t>& pixels){
    std::vector<uint8_t> filtered;for(int y=0;y<size;++y){filtered.push_back(0);filtered.insert(filtered.end(),pixels.begin()+y*size*4,pixels.begin()+(y+1)*size*4);}
    uLongf count=compressBound(filtered.size());std::vector<uint8_t> compressed(count);
    if(compress2(compressed.data(),&count,filtered.data(),filtered.size(),6)!=Z_OK)throw std::runtime_error("PNG compression failed");compressed.resize(count);
    std::vector<uint8_t> out={137,80,78,71,13,10,26,10},header;big(header,size);big(header,size);header.insert(header.end(),{8,6,0,0,0});chunk(out,"IHDR",header);chunk(out,"IDAT",compressed);chunk(out,"IEND",{});
    std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(out.data()),out.size());if(!f)throw std::runtime_error("Unable to write thumbnail");
}
bool head(int group){return group==0 || group==4 || group==6 || group==8 || (group>=10 && group<=18);}
void render(const CatalogAvatar& avatar,const Look& look,int group,const std::filesystem::path& path){
    const auto& model=avatar.model;const auto skin=retarget(model,model,model.pose(1.2f,false));
    std::vector<Texture> textures(model.images.size());
    for(size_t t=0;t<textures.size();++t){auto& tex=textures[t];
        if(auto it=avatar.pixels.find(t);it!=avatar.pixels.end()){tex.w=tex.h=256;tex.pixels=it->second;}
        else {int n;auto* p=stbi_load_from_memory(model.images[t].data(),model.images[t].size(),&tex.w,&tex.h,&n,4);if(!p)throw std::runtime_error("Invalid thumbnail texture");tex.pixels.assign(p,p+tex.w*tex.h*4);stbi_image_free(p);}
    }
    float left=-.5f,right=.5f,top=-1.45f,bottom=.05f;
    if(head(group)){
        bottom=-10;for(const auto& part:model.parts)if(part.slot==1)for(const auto& v:part.vertices){const auto p=projectVertex(v,skin,0,0,1,-.10f,1,1);bottom=std::max(bottom,p.y);}
        bottom+=.035f;left=10;right=-10;top=10;
        for(const auto& part:model.parts)for(const auto& v:part.vertices){const auto p=projectVertex(v,skin,0,0,1,-.10f,1,1);if(p.y<=bottom && std::abs(p.x)<3 && p.y>-5){left=std::min(left,p.x);right=std::max(right,p.x);top=std::min(top,p.y);}}
        left-=.04f;right+=.04f;top-=.04f;bottom+=.02f;
    }
    if(group==1){left=-.46f;right=.46f;top=-1.14f;bottom=-.56f;}
    if(group==2){left=-.34f;right=.34f;top=-.80f;bottom=.04f;}
    if(group==3){left=-.28f;right=.28f;top=-.28f;bottom=.05f;}
    if(group==19){left=-1.1f;right=1.1f;top=-1.75f;bottom=.2f;}
    const float zoom=std::min((size-12)/(right-left),(size-12)/(bottom-top));
    const float ox=size/2.f-(left+right)*zoom/2,oy=size/2.f-(top+bottom)*zoom/2;
    std::vector<uint8_t> output(size*size*4);std::vector<float> depth(size*size,1.f);
    struct V {ProjectedVertex p;std::array<float,3> tint;};
    for(int pass=0;pass<2;++pass)for(const auto& part:model.parts){
        if(int(part.blend)!=pass)continue;std::vector<V> vertices;std::vector<ProjectedVertex> projected;
        const auto dye=part.slot==0?rgb(skinPalette[look.colors[1]]):part.slot==2?rgb(colorPalette[look.colors[2]]):std::array<float,3>{1,1,1};
        for(size_t i=0;i<part.vertices.size();++i){const auto& v=part.vertices[i];auto p=projectVertex(v,skin,0,0,1,-.10f,.9f+.05f*look.colors[8],.88f+.06f*look.colors[9]);p.x=p.x*zoom+ox;p.y=p.y*zoom+oy;
            const float shade=i<part.normals.size()?catalogShade(v,part.normals[i],skin,-.10f):1;const uint32_t c=i<part.colors.size()?part.colors[i]:0xffffffff;
            vertices.push_back({p,{dye[0]*shade*(c&255)/255.f,dye[1]*shade*(c>>8&255)/255.f,dye[2]*shade*(c>>16&255)/255.f}});projected.push_back(p);
        }
        const auto indices=visibleIndices(part,projected);const auto& tex=textures[part.texture];
        for(size_t i=0;i<indices.size();i+=3){const auto& a=vertices[indices[i]];const auto& b=vertices[indices[i+1]];const auto& c=vertices[indices[i+2]];
            const float den=(b.p.y-c.p.y)*(a.p.x-c.p.x)+(c.p.x-b.p.x)*(a.p.y-c.p.y);if(std::abs(den)<1e-7f)continue;
            const int x0=std::max(0,int(std::floor(std::min({a.p.x,b.p.x,c.p.x})))),x1=std::min(size-1,int(std::ceil(std::max({a.p.x,b.p.x,c.p.x}))));
            const int y0=std::max(0,int(std::floor(std::min({a.p.y,b.p.y,c.p.y})))),y1=std::min(size-1,int(std::ceil(std::max({a.p.y,b.p.y,c.p.y}))));
            for(int y=y0;y<=y1;++y)for(int x=x0;x<=x1;++x){
                const float u=((b.p.y-c.p.y)*(x+.5f-c.p.x)+(c.p.x-b.p.x)*(y+.5f-c.p.y))/den;
                const float v=((c.p.y-a.p.y)*(x+.5f-c.p.x)+(a.p.x-c.p.x)*(y+.5f-c.p.y))/den,w=1-u-v;if(u<0 || v<0 || w<0)continue;
                const auto pixel=y*size+x;const float z=u*a.p.z+v*b.p.z+w*c.p.z;if(z>depth[pixel])continue;
                const int tx=std::clamp(int((u*a.p.u+v*b.p.u+w*c.p.u)*tex.w),0,tex.w-1),ty=std::clamp(int((u*a.p.v+v*b.p.v+w*c.p.v)*tex.h),0,tex.h-1);const auto* sample=&tex.pixels[(ty*tex.w+tx)*4];if(!sample[3])continue;
                auto* dst=&output[pixel*4];const float alpha=sample[3]/255.f,old=dst[3]/255.f,out=alpha+old*(1-alpha);if(out<=0)continue;
                for(int k=0;k<3;++k){const float light=u*a.tint[k]+v*b.tint[k]+w*c.tint[k];dst[k]=uint8_t(std::clamp((sample[k]*light*alpha+dst[k]*old*(1-alpha))/out,0.f,255.f));}dst[3]=uint8_t(out*255+.5f);if(!part.blend && alpha>=.5f)depth[pixel]=z;
            }
        }
    }
    size_t visible=0;for(size_t n=3;n<output.size();n+=4)visible+=output[n]>0;if(visible<10)throw std::runtime_error("Empty item thumbnail: "+path.string());png(path,output);
}
}
int main(int argc,char**argv){
    if(argc!=3)return 2;CatalogIndex index;if(!index.load(argv[1]))return 3;const auto output=std::filesystem::path(argv[2]);std::filesystem::create_directories(output);int count=0;
    try{for(int body=0;body<2;++body){Look base;base.value[Character]=body;index.initialize(base);
        auto neutral=buildCatalogAvatar(index,base,decode);if(!neutral.error.empty())throw std::runtime_error(neutral.error);
        render(neutral,base,20,output/("thumb-body-"+std::to_string(body)+".png"));render(neutral,base,10,output/("thumb-face-"+std::to_string(body)+".png"));count+=2;
        for(const auto& item:index.items){if(!(item.bodies&(body+1)) || item.name.find("(Hat)")!=std::string::npos)continue;
            Look look=base;look.items[item.group]=item.key;auto avatar=buildCatalogAvatar(index,look,decode);if(!avatar.error.empty())throw std::runtime_error(item.name+": "+avatar.error);
            render(avatar,look,item.group,output/("thumb-"+item.key+"-"+std::to_string(body)+".png"));++count;
        }
    }}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 4;}
    std::cout<<count<<" original-model thumbnails baked.\n";
}
