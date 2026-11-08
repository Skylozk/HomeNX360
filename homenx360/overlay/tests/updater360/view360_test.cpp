#include "../../projects/homenx360-updater/UpdaterView360.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
using namespace homenx360::updaterui;
int main(int argc,char**argv){
    assert(argc>=2);View view;assert(view.load(std::string(argv[1])+"/"));
    constexpr int stride=width+17;std::vector<uint32_t> storage(32+stride*height+32,0x12345678);State state;state.installed="v0.9.0-preview.1";state.available="v0.9.1-preview.1";state.download=.46f;state.downloadBytes=774*1024*1024ULL;state.megabytesPerSecond=3.7;state.install=.25f;
    for(auto phase:{Phase::Checking,Phase::Downloading,Phase::Verifying,Phase::Preparing,Phase::Installing,Phase::Complete,Phase::Current,Phase::Failed,Phase::Canceled}){
        state.phase=phase;state.running=int(phase)<=4;state.step=std::min(4,int(phase));state.applet=int(phase)%2;
        state.error="GitHub returned HTTP 403. The request was blocked or rate limited. Try again after checking the connection. "+std::string(2000,'x');
        view.draw(storage.data()+32,stride,state,2.7f);
        for(int i=0;i<32;++i)assert(storage[i]==0x12345678 && storage[storage.size()-1-i]==0x12345678);
        for(int y=0;y<height;++y)for(int x=width;x<stride;++x)assert(storage[32+y*stride+x]==0x12345678);
    }
    if(argc>2){state.phase=Phase::Downloading;state.running=true;state.step=1;state.applet=false;view.draw(storage.data()+32,stride,state,1.5f);
        std::ofstream out(argv[2],std::ios::binary);out<<"P6\n1280 720\n255\n";for(int y=0;y<height;++y)for(int x=0;x<width;++x){uint32_t pixel=storage[32+y*stride+x];for(int shift:{0,8,16})out.put(char(pixel>>shift));}}
    std::cout<<"Updater UI renders all nine states with real font/logo assets; long errors and padded-framebuffer bounds passed.\n";
}
