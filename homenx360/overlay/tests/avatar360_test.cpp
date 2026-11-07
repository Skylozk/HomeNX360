#include "../projects/cards/src/Avatar360.hpp"
#include "../projects/cards/src/AvatarModel.hpp"
#include <cassert>
#include <iostream>
using namespace cards::avatar360;
int main(int argc,char** argv){
    assert(argc==2);const std::string a="00000000000000010000000000000002",b="00000000000000030000000000000004";
    assert(validId(a));assert(!validId(std::string(32,'0')));assert(!validId("bad"));assert(!validId(std::string(32,'g')));
    Wardrobe wardrobe;Session s;s.begin(true);s.loaded({{a,"First"},{b,"Second"},{a,"Duplicate"},{"bad","Invalid"}},a);
    assert(s.profiles.size()==2 && s.selected==0 && s.screen==Screen::Picker);
    s.move(1);s.edit(wardrobe);const auto before=forProfile(wardrobe,b);s.draft.cycle(Top,1);s.back();assert(wardrobe.empty());
    s.edit(wardrobe);s.draft.cycle(Character,1);s.draft.cycle(Top,2);const auto saved=s.draft;assert(s.save(wardrobe));assert(wardrobe.size()==1 && wardrobe.at(b)==saved && !wardrobe.count(a));
    assert(forProfile(wardrobe,b)==saved);assert(forProfile(wardrobe,a)==forProfile(wardrobe,a));
    Session canceled;canceled.begin(true);canceled.back();canceled.loaded({{a,"Late result"}},a);assert(!canceled.open());canceled.begin(false);canceled.loaded({{b,"New load"}},b);assert(canceled.current()->id==b);
    auto selected=s.signIn();assert(selected && selected->id==b);s.signedIn();s.tick(1);assert(s.screen==Screen::Welcome);s.tick(5);assert(!s.open());
    Look corrupt;corrupt.value.fill(999);corrupt.normalize();for(int i=0;i<FeatureCount;++i)assert(corrupt.value[i]==choices[i]-1);
    for(int i=0;i<FeatureCount;++i){Look l;l.cycle(i,-1);assert(l.value[i]>=0 && l.value[i]<choices[i]);}
    assert(firstVisible(0,8)==0 && firstVisible(7,8)==4);
    std::array<Model,6> models;
    for(int i=0;i<6;++i){
        assert(loadModel(std::string(argv[1])+"/"+modelFiles[i]+".avb",models[i]));assert(models[i].bones.size()==71);
        for(float time:{0.f,1.2f,4.f,30.f})for(bool wave:{false,true}){
            const auto world=models[i].pose(time,wave);const auto skin=retarget(models[i],models[i],world);assert(skin.size()==71);
            size_t visible=0;float minY=100,maxY=-100;
            for(const auto& part:models[i].parts){
                std::vector<ProjectedVertex> vertices;
                for(const auto& v:part.vertices){auto p=skinVertex(v,skin);assert(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z));assert(std::abs(p.x)<3 && p.y>-.5f && p.y<3 && std::abs(p.z)<3);minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);vertices.push_back(projectVertex(v,skin,640,500,210,.3f,1,1));}
                visible+=visibleIndices(part,vertices).size();
            }
            assert(visible>1000 && minY<.2f && maxY>1.3f);
        }
        for(int v=0;v<3;++v){const int source=donor(i,v);assert(source%2==i%2);assert(retarget(models[source],models[i],models[i].pose(0,false)).size()==71 || !models[source].valid());}
    }
    // All donor rigs must map by bone identity, independent of joint ordering.
    for(int i=0;i<6;++i)for(int n=0;n<3;++n)assert(retarget(models[donor(i,n)],models[i],models[i].pose(0,false)).size()==71);
    // Even four of the most expensive possible outfits must fit the native
    // indexed stream, independently of the ordinary 2D UI vertex allowance.
    size_t largest=0;
    for(int body=0;body<6;++body)for(int hair=0;hair<4;++hair)for(int top=0;top<3;++top)for(int bottom=0;bottom<3;++bottom)for(int shoes=0;shoes<3;++shoes){
        Look l;l.value[Character]=body;l.value[Hair]=hair;l.value[Top]=top;l.value[Bottom]=bottom;l.value[Shoes]=shoes;size_t bytes=0;
        for(int source=0;source<6;++source)for(const auto& part:models[source].parts)if(partEnabled(l,part.slot,source))bytes+=part.vertices.size()*56+part.indices.size()*4+512;
        largest=std::max(largest,bytes);
    }
    assert(largest*4<=4u*1024u*1024u);std::cout<<"Four-avatar stream upper bound: "<<largest*4<<" / 4194304 bytes\n";
    const std::string path=std::string(argv[1])+"/regular.avb";std::ifstream in(path,std::ios::binary);std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)),{});Model untouched=models[0];
    const auto bones=untouched.bones;bytes.resize(100);assert(!decodeModel(bytes,untouched) && untouched.bones==bones);
    bytes.assign(24,0xff);assert(!decodeModel(bytes,untouched));
    std::cout<<"Avatar profiles, per-UID draft/save/cancel, all six rigs, animation bounds, wardrobe mapping and invalid banks passed.\n";
}
