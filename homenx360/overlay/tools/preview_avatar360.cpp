// Export the SAME native CPU skinning/projection used by the Switch frontend.
// tools/render_avatar360.py supplies a software z-buffer for reference images.
#include "../projects/cards/src/Avatar360.hpp"
#include "../projects/cards/src/AvatarModel.hpp"
#include <iostream>
using namespace cards::avatar360;
template<class T> void write(std::ofstream& out,const T& value){out.write(reinterpret_cast<const char*>(&value),sizeof(value));}
int main(int argc,char** argv){
    if(argc<3)return 2;std::array<Model,6> models;
    for(size_t i=0;i<models.size();++i)if(!loadModel(std::string(argv[1])+"/"+modelFiles[i]+".avb",models[i])){std::cerr<<"Invalid model "<<modelFiles[i]<<"\n";return 1;}
    std::ofstream out(argv[2],std::ios::binary);uint32_t imageCount=0,partCount=0;
    for(const auto& model:models)imageCount+=model.images.size();
    write(out,imageCount);const auto countPos=out.tellp();write(out,partCount);
    for(const auto& model:models)for(const auto& png:model.images){write(out,uint32_t(png.size()));out.write(reinterpret_cast<const char*>(png.data()),png.size());}
    const bool editor=argc>3 && std::string(argv[3])=="editor";
    const bool wardrobe=argc>3 && std::string(argv[3])=="wardrobe";
    for(int person=0;person<(editor?1:4);++person){
        Look look;look.value[Character]=std::array<int,4>{1,0,2,5}[person];
        if(wardrobe){look.value[Hair]=1;look.value[HairColour]=3;look.value[Top]=2;look.value[Bottom]=1;look.value[Shoes]=2;}
        const auto& body=models[look.value[Character]];const auto world=body.pose(1.2f,false);
        for(int blend=0;blend<2;++blend){uint32_t imageOffset=0;
            for(int source=0;source<6;++source){
                const auto& model=models[source];const auto skin=retarget(model,body,world);if(skin.empty())return 3;
                for(const auto& part:model.parts){
                    if(int(part.blend)!=blend || !partEnabled(look,part.slot,source))continue;
                    std::vector<ProjectedVertex> vertices;
                    for(const auto& v:part.vertices)vertices.push_back(projectVertex(v,skin,editor?330.f:190.f+335.f*person,editor?575.f:490.f,editor?310.f:210.f,editor?.3f:-.08f,1,1));
                    auto indices=visibleIndices(part,vertices);if(indices.empty())continue;
                    write(out,uint32_t(imageOffset+part.texture));write(out,part.blend);write(out,uint32_t(vertices.size()));write(out,uint32_t(indices.size()));
                    const auto dye=part.slot==2 && look.value[HairColour]?hairTints[look.value[HairColour]]:std::array<float,3>{-1,-1,-1};
                    for(float c:dye)write(out,c);
                    out.write(reinterpret_cast<const char*>(vertices.data()),vertices.size()*sizeof(ProjectedVertex));out.write(reinterpret_cast<const char*>(indices.data()),indices.size()*4);++partCount;
                }
                imageOffset+=model.images.size();
            }
        }
    }
    out.seekp(countPos);write(out,partCount);return !out;
}
