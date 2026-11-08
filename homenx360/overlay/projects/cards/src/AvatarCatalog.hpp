#pragma once
#include "Avatar360.hpp"
#include "AvatarModel.hpp"
#include <functional>
#include <set>

namespace cards::avatar360 {
inline constexpr std::array<uint32_t,21> catalogCategories={4,8,16,32,64,128,256,512,1024,2048,8192,16384,32768,65536,131072,262144,524288,1048576,2097152,4096,0};
enum CatalogField {CBody,CHair,CHairColor,CTop,CBottom,CShoes,COutfit,CHat,CGloves,CGlasses,CWrist,CEarrings,CRing,CEyes,CEyeColor,CBrows,CBrowColor,CMouth,CLipColor,CFacialHair,CFacialColor,CFacePaint,CFaceColor1,CFaceColor2,CShadow,CShadowColor,CNose,CChin,CEars,CSkin,CHeight,CBuild,CProp,CFieldCount};
inline constexpr std::array<const char*,CFieldCount> catalogFieldNames={"Body type","Hairstyle","Hair colour","Tops","Bottoms","Shoes","Outfits & costumes","Headwear","Gloves","Glasses","Wristwear","Earrings","Rings","Eyes","Eye colour","Eyebrows","Eyebrow colour","Mouth","Lip colour","Facial hair","Facial hair colour","Face features","Face feature colour 1","Face feature colour 2","Eye shadow","Eye shadow colour","Nose","Chin","Ears","Skin tone","Height","Build","Props"};
inline int fieldGroup(int field){switch(field){case CHair:return 0;case CTop:return 1;case CBottom:return 2;case CShoes:return 3;case CHat:return 4;case CGloves:return 5;case CGlasses:return 6;case CWrist:return 7;case CEarrings:return 8;case CRing:return 9;case CEyes:return 10;case CBrows:return 11;case CMouth:return 12;case CFacialHair:return 13;case CFacePaint:return 14;case CShadow:return 15;case CNose:return 16;case CChin:return 17;case CEars:return 18;case CProp:return 19;case COutfit:return 20;default:return -1;}}
inline int fieldColor(int field){switch(field){case CBody:return 0;case CSkin:return 1;case CHairColor:return 2;case CEyeColor:return 3;case CBrowColor:return 4;case CLipColor:return 5;case CFacialColor:return 6;case CShadowColor:return 7;case CFaceColor1:return 10;case CFaceColor2:return 11;case CHeight:return 8;case CBuild:return 9;default:return -1;}}
inline constexpr std::array<uint32_t,24> skinPalette={0x3d231b,0x563023,0x72412d,0x8b5338,0xa86c48,0xc48960,0xe0a577,0xedb582,0xf2c498,0xf6d2ac,0xffdfbe,0xffe7cd,0x694b3d,0x82614c,0xa37b5a,0xbb906c,0xd5ad84,0xe4bf9a,0xeecfb0,0xf5dcc2,0x977563,0xbb9780,0xdcbaa4,0xf0d3bd};
inline constexpr std::array<uint32_t,32> colorPalette={0x070706,0x1c130e,0x392514,0x694329,0x916037,0xb88647,0xd6ae61,0xf2d58e,0x6f2415,0xa3401f,0xc2642e,0xe59050,0x635c52,0x8c877e,0xc5c5bf,0xf1eee0,0x27421e,0x487454,0x7a9b52,0x234454,0x447f9f,0x8eb5c2,0x493467,0x8a5b9d,0xad2f50,0xdb718a,0xb65d54,0xdb9c87,0x9f784b,0xe0b77b,0xefd7bb,0xffffff};
inline std::array<float,3> rgb(uint32_t c){return{float(c>>16&255)/255,float(c>>8&255)/255,float(c&255)/255};}
inline int colorCount(int i){return i==0?2:i==1?int(skinPalette.size()):(i==8 || i==9)?5:int(colorPalette.size());}
inline void normalizeCatalog(Look& l){for(int i=0;i<12;++i)l.colors[i]=std::clamp(l.colors[i],0,colorCount(i)-1);for(auto& k:l.items)if(k.size()>40 || k.find_first_not_of("0123456789abcdefm-")!=std::string::npos)k.clear();}
struct CatalogItem {std::string key,name;uint32_t categories=0,bodies=0,partner=0;int group=-1;};
inline int catalogGroup(uint32_t c){
    if(c==4096)return 19;
    if(c>8191 && !(c&8191)){for(int i=10;i<=18;++i)if(c==catalogCategories[i])return i;return -1;}
    c&=8191;if((c&8) && (c&16))return 20;
    if((c&64) && !(c&(8|16)))return 4;
    for(int i=0;i<10;++i)if(c&catalogCategories[i])return i;return -1;
}
struct CatalogIndex {
    std::string path;std::vector<CatalogItem> items;std::array<Model,2> bases;bool ready=false;
    const CatalogItem* find(const std::string& key)const{auto it=std::find_if(items.begin(),items.end(),[&](const auto& i){return i.key==key;});return it==items.end()?nullptr:&*it;}
    std::vector<const CatalogItem*> options(int group,int body)const{
        std::vector<const CatalogItem*> result;const bool required=group==1 || group==2 || group==3 || group==10 || group==11 || group==12;
        if(!required)result.push_back(nullptr);
        for(const auto& i:items)if(i.group==group && (i.bodies&(body+1)) && i.name.find("(Hat)")==std::string::npos)result.push_back(&i);
        return result;
    }
    bool load(const std::string& folder){
        std::ifstream f(folder+"/index.avc",std::ios::binary|std::ios::ate);if(!f || f.tellg()<12 || f.tellg()>16*1024*1024)return false;
        std::vector<uint8_t> b(size_t(f.tellg()));f.seekg(0);if(!f.read(reinterpret_cast<char*>(b.data()),b.size()))return false;
        BankReader r(b);char magic[8];uint32_t count=0;
        if(!r.bytes(magic,8) || std::memcmp(magic,"NXACAT1\0",8) || !r.number(count) || !count || count>50000)return false;
        std::vector<CatalogItem> fresh;std::set<std::string> keys;
        for(uint32_t n=0;n<count;++n){CatalogItem i;uint32_t kl=0,nl=0;
            if(!r.number(i.categories)||!r.number(i.bodies)||!r.number(i.partner)||!r.number(kl)||!r.number(nl)||kl<1||kl>40||nl<1||nl>512||i.bodies<1||i.bodies>3)return false;
            i.key.resize(kl);i.name.resize(nl);if(!r.bytes(i.key.data(),kl)||!r.bytes(i.name.data(),nl)||i.key.find_first_not_of("0123456789abcdefm-")!=std::string::npos||i.name.find('\0')!=std::string::npos||!keys.insert(i.key).second)return false;
            i.group=catalogGroup(i.categories);if(i.group<0)return false;fresh.push_back(std::move(i));
        }
        if(r.remaining() || !loadModel(folder+"/base-1.avb",bases[0]) || !bases[0].valid() || !loadModel(folder+"/base-2.avb",bases[1]) || !bases[1].valid())return false;
        std::stable_sort(fresh.begin(),fresh.end(),[](const auto& a,const auto& b){return a.name<b.name;});items=std::move(fresh);path=folder;ready=true;return true;
    }
    void initialize(Look& l)const{
        if(!l.expanded){l.colors[0]=l.value[Character]%2;l.colors[1]=7;l.colors[2]=2;l.colors[3]=20;l.colors[4]=2;l.colors[5]=26;l.colors[6]=2;l.colors[7]=0;l.colors[8]=l.value[Height];l.colors[9]=l.value[Build];l.expanded=true;}
        normalizeCatalog(l);
        for(int g=0;g<21;++g){const auto opts=options(g,l.colors[0]);const auto* selected=find(l.items[g]);
            if(selected && selected->group==g && (selected->bodies&(l.colors[0]+1)))continue;
            l.items[g]=opts.empty() || !opts[0]?"":opts[0]->key;
        }
    }
    std::string label(const Look& look,int field)const{
        const int g=fieldGroup(field);if(g>=0){const auto* i=find(look.items[g]);return i?i->name:g>=16 && g<=18?"Default":"None";}
        const int c=fieldColor(field);if(c==0)return look.colors[c]?"Female":"Male";
        if(c==8 || c==9)return std::to_string(look.colors[c]+1)+" / 5";
        if(c>=0)return std::to_string(look.colors[c]+1)+" / "+std::to_string(colorCount(c));return {};
    }
    void cycle(Look& l,int field,int delta)const{
        initialize(l);const int g=fieldGroup(field),c=fieldColor(field);
        if(g>=0){auto opts=options(g,l.colors[0]);if(opts.empty())return;int at=0;
            for(size_t i=0;i<opts.size();++i)if((opts[i]?opts[i]->key:"")==l.items[g])at=int(i);
            at=((at+delta)%int(opts.size())+int(opts.size()))%int(opts.size());l.items[g]=opts[at]?opts[at]->key:"";
            // A complete costume owns its covered slots. Editing one of those
            // slots removes the costume and restores the individual wardrobe.
            if(g!=20 && g!=19){
                const auto* item=find(l.items[g]);const uint32_t chosen=item?item->categories:catalogCategories[g];
                const auto* outfit=find(l.items[20]);if(outfit && (outfit->categories&chosen))l.items[20].clear();
                if(g<10)for(int other=0;other<10;++other)if(other!=g){
                    const auto* old=find(l.items[other]);if(!old || !(old->categories&chosen))continue;
                    l.items[other].clear();
                    if(other>=1 && other<=3){auto alternatives=options(other,l.colors[0]);auto plain=std::find_if(alternatives.begin(),alternatives.end(),[&](auto candidate){return candidate && (candidate->categories&8191)==catalogCategories[other];});if(plain!=alternatives.end())l.items[other]=(*plain)->key;}
                }
            }
        }else if(c>=0){const int n=colorCount(c);l.colors[c]=((l.colors[c]+delta)%n+n)%n;if(c==0)initialize(l);}
    }
    std::string counter(const Look& l,int field)const{const int g=fieldGroup(field);if(g<0)return {};const auto opts=options(g,l.colors[0]);size_t at=0;for(size_t i=0;i<opts.size();++i)if((opts[i]?opts[i]->key:"")==l.items[g])at=i;return std::to_string(at+1)+" / "+std::to_string(opts.size());}
};
struct CatalogAvatar {Model model;std::map<size_t,std::vector<uint8_t>> pixels;std::string error;};
using DecodeCatalogImage=std::function<std::vector<uint8_t>(const std::string&)>;
inline bool applyCatalogPatch(Model& model,const std::string& path){
    std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)return true;const auto size=f.tellg();if(size<16 || size>2*1024*1024)return false;
    std::vector<uint8_t> bytes(static_cast<size_t>(size));f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),size))return false;
    BankReader r(bytes);char magic[8];uint32_t nv=0,nt=0;
    if(!r.bytes(magic,8)||std::memcmp(magic,"NXAPTC1\0",8)||!r.number(nv)||!r.number(nt)||nv>40000||nt>50000||uint64_t(nv)*20+uint64_t(nt)*8!=r.remaining())return false;
    struct Change{uint32_t part,vertex;V3 delta;};std::vector<Change> changes;std::vector<std::pair<uint32_t,uint32_t>> hidden;
    for(uint32_t n=0;n<nv;++n){Change c;if(!r.number(c.part)||!r.number(c.vertex)||!r.bytes(&c.delta,12)||c.part>=model.parts.size()||c.vertex>=model.parts[c.part].vertices.size())return false;
        for(float v:{c.delta.x,c.delta.y,c.delta.z})if((std::bit_cast<uint32_t>(v)&0x7f800000u)==0x7f800000u || std::abs(v)>.5f)return false;changes.push_back(c);}
    for(uint32_t n=0;n<nt;++n){uint32_t p=0,t=0;if(!r.number(p)||!r.number(t)||p>=model.parts.size()||t%3||t+2>=model.parts[p].indices.size())return false;hidden.emplace_back(p,t);}
    for(const auto& c:changes)model.parts[c.part].vertices[c.vertex].position=model.parts[c.part].vertices[c.vertex].position+c.delta;
    for(const auto& [p,t]:hidden){auto& i=model.parts[p].indices;i[t+1]=i[t+2]=i[t];}return true;
}
inline std::string lookKey(const Look& look){std::string s;for(auto v:look.colors)s+=std::to_string(v)+":";for(const auto& k:look.items)s+=k+":";return s;}
inline CatalogAvatar buildCatalogAvatar(const CatalogIndex& index,Look look,const DecodeCatalogImage& decode){
    CatalogAvatar out;if(!index.ready){out.error="Install the avatar catalog from the complete update.";return out;}index.initialize(look);
    out.model=index.bases[look.colors[0]];auto& model=out.model;const std::string root=index.path+"/";
    auto fail=[&](const std::string& error){out.error=error;out.model={};return out;};
    std::vector<const CatalogItem*> worn;const auto* outfit=index.find(look.items[20]);uint32_t covered=outfit?outfit->categories&8191:0;
    if(outfit)worn.push_back(outfit);
    for(int g:{1,2,3,4,0,5,6,7,8,9})if(!(catalogCategories[g]&covered))if(const auto* item=index.find(look.items[g])){worn.push_back(item);covered|=item->categories&8191;}
    for(int g=16;g<=18;++g)if(const auto* item=index.find(look.items[g]))if(!applyCatalogPatch(model,root+item->key+"-"+std::to_string(look.colors[0]+1)+".avp"))return fail("Invalid face-shape patch.");
    const bool hat=std::any_of(worn.begin(),worn.end(),[](auto i){return i->categories&64;});
    for(const auto* item:worn){std::string key=item->key;
        if(item->group==0 && hat && item->partner && index.find(std::to_string(item->partner)))key=std::to_string(item->partner);
        if(!applyCatalogPatch(model,root+key+"-"+std::to_string(look.colors[0]+1)+".avp"))return fail("Invalid outfit body patch.");
        Model part;const auto path=root+key+".avb";std::ifstream exists(path,std::ios::binary);if(!exists){if(item->group==0 && item->name.find("Bald")!=std::string::npos)continue;return fail("Missing catalog item: "+item->name);}
        if(!loadModel(path,part) || !part.wearable() || part.bones!=model.bones)return fail("Invalid catalog item: "+item->name);
        const size_t texture=model.images.size();for(auto& p:part.parts){p.texture+=texture;model.parts.push_back(std::move(p));}for(auto& img:part.images)model.images.push_back(std::move(img));
    }
    // The two head halves share shape geometry but use their own projected masks.
    for(int half=0;half<2;++half){
        std::array<std::vector<uint8_t>,6> layers;
        auto get=[&](int layer,int group,const char* fallback){const auto* item=index.find(look.items[group]);const std::string file=item?item->key+"-face-"+std::to_string(half)+".png":fallback?std::string(fallback)+std::to_string(half)+".png":"";
            if(!file.empty())layers[layer]=decode(root+file);return file.empty() || layers[layer].size()==256*256*4;};
        if(!get(0,14,nullptr)||!get(1,15,nullptr)||!get(2,12,nullptr)||!get(3,10,nullptr)||!get(4,13,nullptr)||!get(5,11,nullptr))return fail("Missing or invalid face mask.");
        const auto skin=rgb(skinPalette[look.colors[1]]),feature1=rgb(colorPalette[look.colors[10]]),feature2=rgb(colorPalette[look.colors[11]]);const std::array<std::array<float,3>,5> tint={rgb(colorPalette[look.colors[7]]),rgb(colorPalette[look.colors[5]]),rgb(colorPalette[look.colors[3]]),rgb(colorPalette[look.colors[6]]),rgb(colorPalette[look.colors[4]])};
        std::vector<uint8_t> pixels(256*256*4,255);
        for(size_t p=0;p<256*256;++p){std::array<float,3> c=skin;
            if(!layers[0].empty()){auto* b=&layers[0][p*4];const float a=b[3]/255.f;for(int k=0;k<3;++k)c[k]=skin[k]*(1-a)+(feature1[k]*b[0]/255.f+feature2[k]*b[1]/255.f+skin[k]*b[2]/255.f)*a;}
            for(int l=1;l<6;++l)if(!layers[l].empty()){auto* m=&layers[l][p*4];const float a=m[3]/255.f;for(int k=0;k<3;++k){const float f=tint[l-1][k]*m[0]/255.f+(l==1?0.f:m[1]/255.f+skin[k]*m[2]/255.f);c[k]=c[k]*(1-a)+f*a;}}
            for(int k=0;k<3;++k)pixels[p*4+k]=uint8_t(std::clamp(c[k],0.f,1.f)*255+.5f);
        }
        if(model.parts.size()<3 || model.parts[half+1].slot!=1)return fail("Invalid base head layout.");out.pixels[model.parts[half+1].texture]=std::move(pixels);
    }
    if(const auto* prop=index.find(look.items[19])){
        Model performance;if(!loadModel(root+prop->key+"-prop-"+std::to_string(look.colors[0]+1)+".avb",performance)||!performance.valid()||performance.bones.size()<model.bones.size()||!std::equal(model.bones.begin(),model.bones.end(),performance.bones.begin()))return fail("Invalid prop performance.");
        model.bones=performance.bones;model.inverse=performance.inverse;model.clips=std::move(performance.clips);model.looping=true;
        std::map<uint32_t,uint32_t> textures;
        for(auto& part:performance.parts)if(part.slot==4){
            if(!textures.count(part.texture)){textures[part.texture]=model.images.size();model.images.push_back(std::move(performance.images[part.texture]));}
            part.texture=textures[part.texture];model.parts.push_back(std::move(part));
        }
    }
    return out;
}
inline float catalogShade(const SkinVertex& vertex,V3 normal,const std::vector<Matrix>& skin,float yaw){
    V3 n;for(int k=0;k<4;++k)if(vertex.weights[k]>0){const auto a=skin[vertex.joints[k]].point(normal),b=skin[vertex.joints[k]].point({});n=n+(a+b*(-1))*vertex.weights[k];}
    const float co=std::cos(yaw),si=std::sin(yaw),x=n.x*co+n.z*si,z=-n.x*si+n.z*co;
    const float len=std::sqrt(x*x+n.y*n.y+z*z);if(len<.0001f)return 1;
    const float key=std::max(0.f,(-.45f*x+.60f*n.y+.70f*z)/len)/1.0259f*.5f+.5f;
    const float fill=std::max(0.f,(.70f*x+.15f*n.y+.55f*z)/len)/.9028f;
    return std::min(1.15f,.22f+.72f*key*key+.20f*fill);
}
} // namespace cards::avatar360
