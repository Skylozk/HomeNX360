#pragma once
#include "BootFormat.hpp"
#include <nxui/core/Texture.hpp>
#include <chrono>
#include <future>
#include <memory>
#include <string>
struct Mix_Chunk;
namespace cards {
class BootClip {
public:
    ~BootClip();
    void open(const std::string& path);
    bool update(nxui::GpuDevice& gpu,nxui::Renderer& renderer,bool audioReady,bool sound,int volume);
    void draw(nxui::Renderer& renderer);
    void stop();
private:
    struct Data { std::vector<std::uint8_t> bytes; boot360::Bank bank; };
    struct Frame { unsigned index=0; std::vector<std::uint8_t> pixels; };
    using Clock=std::chrono::steady_clock;
    std::future<std::shared_ptr<Data>> m_loading;
    std::future<Frame> m_decoding;
    std::shared_ptr<Data> m_data;
    Frame m_ready;
    std::unique_ptr<nxui::Texture> m_texture;
    Mix_Chunk* m_sound=nullptr;
    Clock::time_point m_opened{},m_started{};
    bool m_active=false,m_playing=false,m_haveReady=false;
    unsigned m_shown=0;
    void queue(unsigned frame);
};
} // namespace cards
