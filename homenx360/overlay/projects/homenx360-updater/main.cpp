#include "ReleaseUpdater360.hpp"
#include <switch.h>
#include <atomic>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <future>
#include <string>
#include <switchu/file_log.hpp>
extern "C" {size_t __nx_heap_size=0x06000000;}
extern "C" void userAppInit(){timeInitialize();}
extern "C" void userAppExit(){timeExit();}
using switchu::manager::ReleaseUpdater;
using switchu::manager::UpdateWorkerStage;
struct Outcome {bool success=false,changed=false;std::string version,error;};
int main(int,char**){
    consoleInit(nullptr);switchu::FileLog::open("homenx360-updater");
    ReleaseUpdater::applyPending();padConfigureInput(1,HidNpadStyleSet_NpadStandard);PadState pad;padInitializeDefault(&pad);
    std::atomic<float> download{0},install{0};std::atomic<int> stage{0};std::future<Outcome> job;
    bool running=false,exitAfter=false;Outcome result;std::string current=ReleaseUpdater::installedVersion();
    auto start=[&](){ReleaseUpdater::clearCancel();download=0;install=0;stage=0;running=true;result={};
        job=std::async(std::launch::async,[&](){Outcome out;try{
            const auto latest=ReleaseUpdater::checkLatest();out.version=latest.version;
            if(!latest.updateAvailable){out.success=true;return out;}
            std::error_code ec;const auto space=std::filesystem::space("sdmc:/",ec);
            const uint64_t required=latest.downloadSize*4+512ULL*1024*1024;
            if(!ec && space.available<required)throw std::runtime_error("Not enough free SD space for download, staging and backup");
            const bool disabled=std::filesystem::exists("sdmc:/atmosphere/contents/0100000000001000/exefs.nsp.disabled") && !std::filesystem::exists("sdmc:/atmosphere/contents/0100000000001000/exefs.nsp");
            const auto update=ReleaseUpdater::install(latest,disabled,download,install,stage);
            out.success=update.success;out.changed=update.success;out.error=update.error;
        }catch(const std::exception& e){out.error=e.what();}catch(...){out.error="Unknown update error";}return out;});
    };
    start();int frame=0;
    while(appletMainLoop()){
        padUpdate(&pad);const auto keys=padGetButtonsDown(&pad);
        if(keys & HidNpadButton_B){
            if(!running)break;
            if(stage.load()!=int(UpdateWorkerStage::Installing)){ReleaseUpdater::requestCancel();exitAfter=true;}
        }
        if(!running && (keys&HidNpadButton_A)){current=ReleaseUpdater::installedVersion();start();}
        if(running && job.wait_for(std::chrono::seconds(0))==std::future_status::ready){result=job.get();running=false;if(exitAfter)break;}
        if(frame++%6==0){consoleClear();
            std::printf("\x1b[32;1mHomeNX360 Updater\x1b[0m\n\n");
            std::printf("Automatically updates the preview app and HomeNX dashboard.\nSettings and saved avatars are preserved.\n\nInstalled: %s\n\n",current.c_str());
            if(running){const char* labels[]={"Checking releases...","Downloading update...","Verifying download...","Extracting update...","Installing files. Keep the console powered on..."};const int at=std::clamp(stage.load(),0,4);std::printf("%s\nDownload: %3.0f%%   Install: %3.0f%%\n",labels[at],download.load()*100,install.load()*100);if(exitAfter)std::printf("Canceling safely...\n");else if(at!=4)std::printf("\nB: Cancel / Exit\n");}
            else if(!result.success)std::printf("Update stopped:\n%s\n\nA: Retry   B: Exit\n",result.error.c_str());
            else if(result.changed)std::printf("Installed %s.\n\nPreview: switch/HomeNX360/HomeNX360.nro\nStandard existing preview copies were updated too.\nRestart the console to load the updated HOME dashboard.\n\nA: Check again   B: Exit\n",result.version.c_str());
            else std::printf("Already up to date (%s).\n\nA: Check again   B: Exit\n",result.version.c_str());
        }
        consoleUpdate(nullptr);
    }
    if(running){if(stage.load()!=int(UpdateWorkerStage::Installing))ReleaseUpdater::requestCancel();job.wait();}
    ReleaseUpdater::shutdownNetwork();switchu::FileLog::close();consoleExit(nullptr);return 0;
}
