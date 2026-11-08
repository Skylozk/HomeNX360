#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#include <zlib.h>

// Native, bounded counterpart of tools/prepare_avatar360.py. This contains no
// Microsoft renderer code; meshes and media retain their separate attribution.
namespace cards::avatar360 {
struct V3 {float x=0,y=0,z=0;V3 operator+(V3 b)const{return{x+b.x,y+b.y,z+b.z};}V3 operator*(float f)const{return{x*f,y*f,z*f};}};
struct Matrix {
    std::array<float,12> m{}; // row-major affine 3x4
    V3 point(V3 p)const{return {m[0]*p.x+m[1]*p.y+m[2]*p.z+m[3],m[4]*p.x+m[5]*p.y+m[6]*p.z+m[7],m[8]*p.x+m[9]*p.y+m[10]*p.z+m[11]};}
};
inline Matrix multiply(const Matrix& a,const Matrix& b){
    Matrix out;
    for(int r=0;r<3;++r)for(int c=0;c<4;++c){
        float v=c==3?a.m[r*4+3]:0;
        for(int k=0;k<3;++k)v+=a.m[r*4+k]*b.m[k*4+c];
        out.m[r*4+c]=v;
    }
    return out;
}
struct SkinVertex {V3 position;float u,v;std::array<float,4> weights;std::array<uint8_t,4> joints;};
static_assert(sizeof(SkinVertex)==40);
struct Part {uint32_t slot=0,texture=0,blend=0;std::vector<SkinVertex> vertices;std::vector<uint32_t> indices;std::vector<V3> normals;std::vector<uint32_t> colors;};
struct Clip {uint32_t id=0,frames=0;float duration=0;std::vector<Matrix> world;};
struct Model {
    std::vector<uint32_t> bones;
    std::vector<Matrix> inverse;
    std::vector<Part> parts;
    std::vector<std::vector<uint8_t>> images;
    std::vector<Clip> clips;
    bool looping=false;
    bool valid()const{return !bones.empty() && !parts.empty() && clips.size()==2;}
    bool wearable()const{return !bones.empty() && !parts.empty() && (clips.empty() || clips.size()==2);}
    std::vector<Matrix> pose(float seconds,bool wave)const{
        if(!valid())return {};
        const auto& clip=clips[wave?1:0];
        // Hold the original relaxed standing pose between idle gestures, as
        // the NXE profile picker does, instead of fidgeting continuously.
        const float pause=looping?0.f:12.f;
        const float cycle=std::fmod(std::max(0.f,seconds),clip.duration+pause);
        const float t=wave?std::clamp(seconds,0.f,clip.duration):std::max(0.f,cycle-pause);
        const float at=t/clip.duration*(clip.frames-1);const auto lo=std::min(uint32_t(at),clip.frames-1),hi=std::min(lo+1,clip.frames-1);const float u=at-lo;
        std::vector<Matrix> result(bones.size());
        for(size_t j=0;j<bones.size();++j)for(int k=0;k<12;++k){const auto a=clip.world[lo*bones.size()+j].m[k],b=clip.world[hi*bones.size()+j].m[k];result[j].m[k]=a+(b-a)*u;}
        return result;
    }
};
class BankReader {
    const uint8_t* p;size_t left;
public:
    explicit BankReader(const std::vector<uint8_t>& data):p(data.data()),left(data.size()){}
    bool bytes(void* dst,size_t n){if(n>left)return false;if(n)std::memcpy(dst,p,n);p+=n;left-=n;return true;}
    template<class T> bool number(T& out){return bytes(&out,sizeof(T));}
    size_t remaining()const{return left;}
};
inline bool decodeUncompressedModel(const std::vector<uint8_t>& data,Model& model){
    if(data.size()>8*1024*1024)return false;
    BankReader r(data);char magic[8];uint32_t joints=0,parts=0,images=0,clips=0;
    if(!r.bytes(magic,8))return false;
    const bool half=!std::memcmp(magic,"NXAVTR3\0",8);
    const bool catalog=half || !std::memcmp(magic,"NXAVTR2\0",8);
    if((!catalog && std::memcmp(magic,"NXAVTR1\0",8)) || !r.number(joints) || !r.number(parts) || !r.number(images) || !r.number(clips) || joints<1 || joints>(catalog?256u:128u) || parts<1 || parts>32 || images<1 || images>32 || (clips!=2 && !(catalog && clips==0)))return false;
    Model out;out.bones.resize(joints);out.inverse.resize(joints);
    auto finite=[](const auto& values){for(float x:values)if((std::bit_cast<uint32_t>(x)&0x7f800000u)==0x7f800000u || std::abs(x)>10000)return false;return true;};
    for(size_t j=0;j<joints;++j){
        std::array<float,16> m;
        if(!r.number(out.bones[j]) || !r.bytes(m.data(),sizeof(m)) || !finite(m))return false;
        if(std::find(out.bones.begin(),out.bones.begin()+j,out.bones[j])!=out.bones.begin()+j)return false;
        for(int row=0;row<3;++row)for(int c=0;c<4;++c)out.inverse[j].m[row*4+c]=m[c*4+row];
    }
    size_t totalVertices=0,totalIndices=0;
    for(uint32_t n=0;n<parts;++n){
        Part part;uint32_t nv=0,ni=0;
        if(!r.number(part.slot) || !r.number(part.texture) || !r.number(nv) || !r.number(ni) || !r.number(part.blend) || part.blend>1 || part.slot>17 || part.texture>=images || nv<1 || nv>20000 || ni<3 || ni>90000 || ni%3)return false;
        totalVertices+=nv;totalIndices+=ni;if(totalVertices>40000 || totalIndices>150000 || uint64_t(nv)*40+uint64_t(ni)*4>r.remaining())return false;
        part.vertices.resize(nv);part.indices.resize(ni);
        if(!r.bytes(part.vertices.data(),nv*sizeof(SkinVertex)))return false;
        if(catalog){
            part.normals.resize(nv);part.colors.resize(nv);
            for(uint32_t i=0;i<nv;++i)if(!r.bytes(&part.normals[i],12) || !finite(std::array<float,3>{part.normals[i].x,part.normals[i].y,part.normals[i].z}) || !r.number(part.colors[i]))return false;
        }
        if(!r.bytes(part.indices.data(),ni*4))return false;
        for(const auto& v:part.vertices){
            if(!finite(std::array<float,5>{v.position.x,v.position.y,v.position.z,v.u,v.v}) || !finite(v.weights))return false;
            float sum=0;for(int k=0;k<4;++k){if(v.joints[k]>=joints || v.weights[k]<0 || v.weights[k]>1.001f)return false;sum+=v.weights[k];}if(std::abs(sum-1)>0.01f)return false;
        }
        for(auto i:part.indices)if(i>=nv)return false;
        out.parts.push_back(std::move(part));
    }
    for(uint32_t n=0;n<images;++n){uint32_t size=0;
        if(!r.number(size) || size<24 || size>1024*1024 || size>r.remaining())return false;
        std::vector<uint8_t> png(size);if(!r.bytes(png.data(),size) || std::memcmp(png.data(),"\x89PNG\r\n\x1a\n",8))return false;
        auto dimension=[&](size_t i){return (uint32_t(png[i])<<24)|(uint32_t(png[i+1])<<16)|(uint32_t(png[i+2])<<8)|png[i+3];};
        if(!dimension(16) || dimension(16)>384 || !dimension(20) || dimension(20)>384)return false;
        out.images.push_back(std::move(png));
    }
    for(uint32_t n=0;n<clips;++n){Clip clip;
        if(!r.number(clip.id) || !r.number(clip.frames) || !r.number(clip.duration) || clip.id!=n || clip.frames<2 || clip.frames>901 || !finite(std::array<float,1>{clip.duration}) || clip.duration<=0 || clip.duration>60 || uint64_t(clip.frames)*joints*(half?24:sizeof(Matrix))>r.remaining())return false;
        clip.world.resize(clip.frames*joints);
        if(half){for(auto& m:clip.world)for(float& v:m.m){uint16_t h=0;if(!r.number(h))return false;const uint32_t sign=uint32_t(h&0x8000)<<16,exponent=(h>>10)&31,fraction=h&1023;
            if(!exponent)v=(h&0x8000?-1.f:1.f)*std::ldexp(float(fraction),-24);
            else v=std::bit_cast<float>(sign|((exponent==31?255:exponent+112)<<23)|(fraction<<13));}}
        else if(!r.bytes(clip.world.data(),clip.world.size()*sizeof(Matrix)))return false;
        for(const auto& m:clip.world)if(!finite(m.m))return false;
        out.clips.push_back(std::move(clip));
    }
    if(r.remaining())return false;
    model=std::move(out);return true;
}
inline bool decodeModel(const std::vector<uint8_t>& data,Model& model){
    if(data.size()>=12 && !std::memcmp(data.data(),"NXAZIP1\0",8)){
        uint32_t size=0;std::memcpy(&size,data.data()+8,4);
        if(size<24 || size>8*1024*1024 || data.size()>8*1024*1024)return false;
        std::vector<uint8_t> plain(size);uLongf length=size;uLong packed=data.size()-12;
        if(uncompress2(plain.data(),&length,data.data()+12,&packed)!=Z_OK || length!=size || packed!=data.size()-12)return false;
        return decodeUncompressedModel(plain,model);
    }
    return decodeUncompressedModel(data,model);
}
inline bool loadModel(const std::string& path,Model& model){
    std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)return false;const auto size=f.tellg();
    if(size<24 || size>8*1024*1024)return false;
    std::vector<uint8_t> data(static_cast<size_t>(size));f.seekg(0);if(!f.read(reinterpret_cast<char*>(data.data()),size))return false;
    return decodeModel(data,model);
}
inline std::vector<Matrix> retarget(const Model& source,const Model& body,const std::vector<Matrix>& world){
    std::vector<Matrix> skin(source.bones.size());
    for(size_t j=0;j<source.bones.size();++j){
        const auto it=std::find(body.bones.begin(),body.bones.end(),source.bones[j]);
        if(it==body.bones.end())return {};
        skin[j]=multiply(world[it-body.bones.begin()],source.inverse[j]);
    }
    return skin;
}
inline V3 skinVertex(const SkinVertex& vertex,const std::vector<Matrix>& skin){
    V3 p;for(int k=0;k<4;++k)if(vertex.weights[k]>0)p=p+skin[vertex.joints[k]].point(vertex.position)*vertex.weights[k];return p;
}
struct ProjectedVertex {float x,y,z,u,v;};
inline ProjectedVertex projectVertex(const SkinVertex& v,const std::vector<Matrix>& skin,float x,float base,float scale,float yaw,float height,float build,bool portrait=false){
    V3 p=skinVertex(v,skin);p.x*=build;p.y*=height;
    const float co=std::cos(yaw),si=std::sin(yaw),rx=p.x*co+p.z*si,rz=-p.x*si+p.z*co;
    // Orthographic camera matches the NXE sign-in composition. Positive model
    // Z faces the viewer; depth zero is nearest and one is farthest.
    return {x+rx*scale,base-(p.y-(portrait?1.32f*height:0.f))*scale,std::clamp(.5f-rz*.16f,.01f,.99f),v.u,v.v};
}
inline std::vector<uint32_t> visibleIndices(const Part& part,const std::vector<ProjectedVertex>& vertices){
    std::vector<uint32_t> indices;indices.reserve(part.indices.size());
    std::vector<std::pair<float,size_t>> triangles;
    for(size_t i=0;i<part.indices.size();i+=3){
        const auto a=part.indices[i],b=part.indices[i+1],c=part.indices[i+2];
        const auto& p=vertices[a];const auto& q=vertices[b];const auto& s=vertices[c];
        if((q.x-p.x)*(s.y-p.y)-(q.y-p.y)*(s.x-p.x)>=0)continue;
        if(part.blend)triangles.push_back({(p.z+q.z+s.z)/3,i});else indices.insert(indices.end(),{a,b,c});
    }
    if(part.blend){std::stable_sort(triangles.begin(),triangles.end(),[](auto a,auto b){return a.first>b.first;});for(auto [depth,i]:triangles)indices.insert(indices.end(),part.indices.begin()+i,part.indices.begin()+i+3);}
    return indices;
}
} // namespace cards::avatar360
