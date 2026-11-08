#include "../projects/cards/src/Wallpaper360.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <nxui/third_party/stb/stb_image.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
int main(int argc,char**argv){
    assert(argc==2);using namespace cards::wallpaper360;
    const auto root=std::filesystem::path(argv[1]);std::set<std::string> ids,files;
    for(const auto& entry:entries){
        assert(ids.insert(entry.id).second && files.insert(entry.file).second);
        const auto saved=selection(entry);assert(find(saved)==&entry && label(saved)==entry.name);
        assert(resolve(saved,"romfs:/")==std::string("romfs:/wallpapers/")+entry.file);
        assert(resolve(saved,"sdmc:/switch/SwitchU/cards/")==std::string("sdmc:/switch/SwitchU/cards/wallpapers/")+entry.file);
        std::ifstream stream(root/entry.file,std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(stream)),{});
        int width=0,height=0,channels=0;auto* pixels=stbi_load_from_memory(bytes.data(),int(bytes.size()),&width,&height,&channels,4);
        assert(pixels && width==1280 && height==720);stbi_image_free(pixels);
    }
    assert(resolve("nxe:../../other","romfs:/").empty());assert(resolve("nxe:removed","romfs:/").empty());
    assert(resolve("sdmc:/config/CardsNX/backgrounds/mine.jpg","romfs:/")=="sdmc:/config/CardsNX/backgrounds/mine.jpg");
    assert(resolve("","romfs:/").empty() && label("")=="Classic NXE");
    std::cout<<"Eleven wallpaper images decode with the native image library; stable app/HOME IDs and fallback paths passed.\n";
}
