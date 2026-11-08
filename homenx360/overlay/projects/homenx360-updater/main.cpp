#include "ReleaseUpdater360.hpp"
#include "UpdaterView360.hpp"
#include <switch.h>
#include <atomic>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <future>
#include <mutex>
#include <string>
#include <switchu/file_log.hpp>
// Keep libnx's adaptive heap for both applet and application launches.
extern "C" void userAppInit(){timeInitialize();}
extern "C" void userAppExit(){timeExit();}
using switchu::manager::ReleaseUpdater;
using switchu::manager::UpdateWorkerStage;
namespace ui=homenx360::updaterui;
struct Outcome {bool success=false,changed=false,canceled=false;int failedStage=0;std::string version,error;};
static std::string installed(){try{return ReleaseUpdater::installedVersion();}catch(const std::exception& error){switchu::FileLog::log("[ui] reading installed version: %s",error.what());return "unknown";}}
int main(int,char**){
    switchu::FileLog::open("homenx360-updater");
    const bool mounted=R_SUCCEEDED(romfsInit());ui::View view;Framebuffer fb{};bool graphical=false;
    try{if(mounted && view.load("romfs:/")){
        const Result created=framebufferCreate(&fb,nwindowGetDefault(),ui::width,ui::height,PIXEL_FORMAT_RGBA_8888,2);
        if(R_SUCCEEDED(created)){const Result linear=framebufferMakeLinear(&fb);graphical=R_SUCCEEDED(linear);if(!graphical)framebufferClose(&fb);}
    }}catch(...){if(fb.has_init)framebufferClose(&fb);}
    if(!graphical){view=ui::View{};consoleInit(nullptr);switchu::FileLog::log("[ui] graphics unavailable; using console fallback");}
    try{ReleaseUpdater::applyPending();}catch(const std::exception& error){switchu::FileLog::log("[ui] pending files: %s",error.what());}
    padConfigureInput(1,HidNpadStyleSet_NpadStandard);PadState pad;padInitializeDefault(&pad);
    std::atomic<float> download{0},install{0};std::atomic<int> stage{0};std::atomic<uint64_t> total{0};std::future<Outcome> job;
    std::mutex versionMutex;std::string available;bool running=false,canceling=false,exitAfter=false;Outcome result;std::string current=installed();
    auto start=[&](){ReleaseUpdater::clearCancel();download=0;install=0;stage=0;total=0;running=true;canceling=exitAfter=false;result={};
        {std::lock_guard<std::mutex> lock(versionMutex);available.clear();}
        try{job=std::async(std::launch::async,[&](){Outcome out;try{
            const auto latest=ReleaseUpdater::checkLatest();out.version=latest.version;total=latest.downloadSize;
            {std::lock_guard<std::mutex> lock(versionMutex);available=latest.version;}
            if(!latest.updateAvailable){out.success=true;return out;}
            std::error_code ec;const auto space=std::filesystem::space("sdmc:/",ec);
            const uint64_t required=latest.downloadSize*4+512ULL*1024*1024;
            if(!ec && space.available<required){char message[192];std::snprintf(message,sizeof(message),"Not enough SD space: %.1f GB free; about %.1f GB required for download, staging and backup.",space.available/1073741824.0,required/1073741824.0);throw std::runtime_error(message);}
            const bool disabled=std::filesystem::exists("sdmc:/atmosphere/contents/0100000000001000/exefs.nsp.disabled") && !std::filesystem::exists("sdmc:/atmosphere/contents/0100000000001000/exefs.nsp");
            const auto update=ReleaseUpdater::install(latest,disabled,download,install,stage);
            out.success=update.success;out.changed=update.success;out.error=update.error;out.failedStage=update.failedStage;
        }catch(const std::exception& error){out.error=error.what();out.failedStage=stage.load();}catch(...){out.error="Unknown update error";out.failedStage=stage.load();}return out;});
        }catch(const std::exception& error){running=false;result.error=std::string("Could not start the update worker: ")+error.what();}
    };
    start();int frame=0;const auto started=std::chrono::steady_clock::now();auto sampled=started;float previousDownload=0;double speed=0;int previousStage=-1;
    while(appletMainLoop()){
        padUpdate(&pad);const auto keys=padGetButtonsDown(&pad);
        if(keys & HidNpadButton_B){if(!running)break;
            if(stage.load()!=int(UpdateWorkerStage::Installing)){if(canceling)exitAfter=true;canceling=true;ReleaseUpdater::requestCancel();}}
        if(!running && (keys&HidNpadButton_A)){current=installed();start();previousStage=-1;speed=0;}
        if(running && job.wait_for(std::chrono::seconds(0))==std::future_status::ready){result=job.get();result.canceled=canceling && !result.success;running=false;if(result.success && result.changed)current=result.version;if(exitAfter)break;}
        const auto now=std::chrono::steady_clock::now();const int at=std::clamp(stage.load(),0,4);const float progress=download.load();
        if(previousStage!=at){previousStage=at;sampled=now;previousDownload=progress;speed=0;}
        const double elapsed=std::chrono::duration<double>(now-sampled).count();
        if(at==1 && elapsed>=.5){const double sample=std::max(0.f,progress-previousDownload)*total.load()/1048576.0/elapsed;speed=speed*.65+sample*.35;sampled=now;previousDownload=progress;}
        std::string version;{std::lock_guard<std::mutex> lock(versionMutex);version=available;}
        if(graphical){ui::State state;state.running=running;state.canceling=canceling;state.applet=appletGetAppletType()!=AppletType_Application && appletGetAppletType()!=AppletType_SystemApplication;
            state.installed=current;state.available=version;state.download=progress;state.install=install.load();state.downloadBytes=total.load();state.megabytesPerSecond=speed;state.step=running?at:result.failedStage;
            state.phase=running?ui::Phase(at):result.canceled?ui::Phase::Canceled:!result.success?ui::Phase::Failed:result.changed?ui::Phase::Complete:ui::Phase::Current;state.error=result.error;
            u32 stride=0;auto* pixels=static_cast<uint32_t*>(framebufferBegin(&fb,&stride));view.draw(pixels,stride/4,state,std::chrono::duration<float>(now-started).count());framebufferEnd(&fb);
        }else{
            if(frame++%6==0){consoleClear();std::printf("HomeNX360 Updater\n\nUpdates the preview app and HOME dashboard.\nSettings and saved avatars are preserved.\n\nInstalled: %s\nAvailable: %s\n\n",current.c_str(),version.c_str());
                if(running){const char* labels[]={"Checking releases...","Downloading update...","Verifying download...","Preparing update...","Installing. Keep the console powered on..."};std::printf("%s\nDownload: %3.0f%%   Install: %3.0f%%\n",labels[at],progress*100,install.load()*100);if(at!=4)std::printf("B: Pause / Exit\n");}
                else if(!result.success)std::printf("%s\n\nA: Retry   B: Exit\n",result.error.c_str());
                else if(result.changed)std::printf("Installed %s. Restart the console for HOME.\n\nA: Check again   B: Exit\n",result.version.c_str());
                else std::printf("Already up to date.\n\nA: Check again   B: Exit\n");}
            consoleUpdate(nullptr);
        }
    }
    if(running){if(stage.load()!=int(UpdateWorkerStage::Installing))ReleaseUpdater::requestCancel();job.wait();}
    ReleaseUpdater::shutdownNetwork();switchu::FileLog::close();if(graphical)framebufferClose(&fb);else consoleExit(nullptr);if(mounted)romfsExit();return 0;
}
