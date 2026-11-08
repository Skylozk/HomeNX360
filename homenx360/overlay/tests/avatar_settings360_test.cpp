#include "../projects/cards/src/Settings.hpp"
#include <cassert>
#include <iostream>
namespace cards {void writeLater(const std::string& path,std::string data){std::filesystem::create_directories(std::filesystem::path(path).parent_path());std::ofstream(path)<<data;}void flushWrites(){}}
int main(){
    const auto previous=std::filesystem::current_path();const auto tmp=std::filesystem::temp_directory_path()/"homenx360-avatar-settings-test";std::filesystem::create_directories(tmp);std::filesystem::current_path(tmp);
    const std::string id="00000000000000010000000000000002";
    cards::Settings old;auto& legacy=old.profileAvatars[id];legacy.value[0]=5;legacy.value[3]=2;const auto before=legacy;old.save();cards::Settings restored;restored.load();assert(restored.profileAvatars.at(id)==before && !restored.profileAvatars.at(id).expanded);
    auto& look=restored.profileAvatars[id];look.expanded=true;look.items[20]="m-00000ffc762d3291cd9d9b6d5553085d";look.colors[0]=1;look.colors[1]=17;look.colors[2]=9;restored.save();cards::Settings again;again.load();assert(again.profileAvatars.at(id)==look);
    std::filesystem::current_path(previous);std::filesystem::remove_all(tmp);std::cout<<"Legacy and full catalog settings round trips passed.\n";
}
