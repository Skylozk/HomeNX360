#include "BootClip.hpp"
#include <nxui/core/Renderer.hpp>
#include <nxui/core/ThreadPool.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <webp/decode.h>
#include <switch.h>
#include <fstream>
namespace cards {
BootClip::~BootClip() { stop(); }
void BootClip::open(const std::string& path) {
    m_active=true;m_playing=false;m_opened=Clock::now();
    m_loading=std::async(std::launch::async,[path]() {
        nxui::moveToWorkerCores();svcSetThreadPriority(CUR_THREAD_HANDLE,0x3B);
        auto data=std::make_shared<Data>();
        std::ifstream file(path,std::ios::binary|std::ios::ate);
        const auto size=file.tellg();
        if (!file || size<40 || size>96*1024*1024) throw std::runtime_error("Startup asset missing or invalid");
        data->bytes.resize(static_cast<size_t>(size));file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(data->bytes.data()),size)) throw std::runtime_error("Startup asset truncated");
        data->bank=boot360::parse(data->bytes);
        return data;
    });
}
void BootClip::queue(unsigned index) {
    if (m_decoding.valid() || !m_data || index>=m_data->bank.count) return;
    auto data=m_data;
    m_decoding=std::async(std::launch::async,[data,index]() {
        nxui::moveToWorkerCores();svcSetThreadPriority(CUR_THREAD_HANDLE,0x3B);
        const auto& b=data->bank;
        const auto* src=data->bytes.data()+b.offsets[index];
        const auto size=b.offsets[index+1]-b.offsets[index];
        int w=0,h=0;
        if (!WebPGetInfo(src,size,&w,&h) || w!=int(b.width) || h!=int(b.height)) throw std::runtime_error("Invalid startup image");
        Frame frame;frame.index=index;frame.pixels.resize(size_t(w)*h*4);
        if (!WebPDecodeRGBAInto(src,size,frame.pixels.data(),frame.pixels.size(),w*4)) throw std::runtime_error("Startup image decode failed");
        return frame;
    });
}
bool BootClip::update(nxui::GpuDevice& gpu,nxui::Renderer& renderer,bool audioReady,bool sound,int volume) {
    using namespace std::chrono_literals;
    try {
        // A skipped startup's jobs finish off-thread, then release their buffers.
        if (!m_active) {
            if (m_loading.valid() && m_loading.wait_for(0s)==std::future_status::ready) (void)m_loading.get();
            if (m_decoding.valid() && m_decoding.wait_for(0s)==std::future_status::ready) (void)m_decoding.get();
            return false;
        }
        if (m_loading.valid() && m_loading.wait_for(0s)==std::future_status::ready) { m_data=m_loading.get();queue(0); }
        if (!m_data) return true;
        if (m_decoding.valid() && m_decoding.wait_for(0s)==std::future_status::ready) { m_ready=m_decoding.get();m_haveReady=true; }
        const auto now=Clock::now();
        bool starting=false;
        if (!m_playing && m_haveReady) {
            // Start image and audio together. Audio failure must not trap HOME.
            if (sound && !audioReady && now-m_opened<3s) return true;
            if (audioReady && sound && m_data->bank.audioSize) {
                auto* rw=SDL_RWFromConstMem(m_data->bytes.data()+m_data->bank.audioOffset,m_data->bank.audioSize);
                if (rw) m_sound=Mix_LoadWAV_RW(rw,1);
                if (m_sound) Mix_Volume(0,MIX_MAX_VOLUME*std::clamp(volume,0,100)/100);
            }
            m_started=now;m_playing=true;starting=true;
        }
        if (!m_playing) return true;
        const double elapsed=std::chrono::duration<double>(now-m_started).count();
        if (elapsed>=m_data->bank.duration()) { stop();return false; }
        const unsigned wanted=m_data->bank.frameAt(elapsed);
        if (m_haveReady && m_ready.index<=wanted) {
            auto texture=std::make_unique<nxui::Texture>();
            nxui::GpuDevice::EssentialScope essential;
            if (!texture->loadFromPixels(gpu,renderer,m_ready.pixels.data(),m_data->bank.width,m_data->bank.height)) {
                stop();return false;
            }
            m_texture=std::move(texture);m_shown=m_ready.index;m_haveReady=false;m_ready.pixels.clear();
            if (starting) {
                m_started=Clock::now();
                if (m_sound) Mix_PlayChannel(0,m_sound,0);
            }
            // Follow real elapsed time if decoding ever falls behind the sound.
            queue(std::max(wanted,m_shown)+1);
        }
        return true;
    } catch (const std::exception&) { stop();return false; }
}
void BootClip::draw(nxui::Renderer& renderer) {
    renderer.drawRect({0,0,1280,720},{0,0,0,1});
    if (m_texture) renderer.drawTexture(m_texture.get(),{0,0,1280,720});
}
void BootClip::stop() {
    m_active=false;m_playing=false;
    if (m_sound) { Mix_HaltChannel(0);Mix_FreeChunk(m_sound);m_sound=nullptr; }
    m_texture.reset();m_data.reset();m_ready={};m_haveReady=false;
}
} // namespace cards
