#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <string>
namespace cards::boot360 {
struct SkipGate {
    bool armed=false;
    bool consume(bool held,bool down) {
        if (!held) armed=true;
        return armed && down;
    }
};
struct Bank {
    std::uint32_t width=0,height=0,num=0,den=0,count=0,durationUs=0,audioSize=0,audioOffset=0;
    std::vector<std::uint32_t> offsets;
    unsigned frameAt(double seconds) const {
        return static_cast<unsigned>(std::clamp(std::floor(seconds*num/den),0.,double(count-1)));
    }
    double duration() const { return durationUs/1000000.; }
};
inline Bank parsePrefix(const std::vector<std::uint8_t>& bytes,std::uint64_t fileSize) {
    const char magic[8]={'N','X','B','O','O','T','1','\0'};
    if (bytes.size()<40 || fileSize>96u*1024u*1024u || !std::equal(magic,magic+8,bytes.begin()))
        throw std::runtime_error("Invalid startup bank");
    auto u32=[&](size_t p) { return std::uint32_t(bytes.at(p)) | std::uint32_t(bytes.at(p+1))<<8 |
        std::uint32_t(bytes.at(p+2))<<16 | std::uint32_t(bytes.at(p+3))<<24; };
    Bank b; b.width=u32(8);b.height=u32(12);b.num=u32(16);b.den=u32(20);b.count=u32(24);
    b.durationUs=u32(28);b.audioSize=u32(32);
    if (b.width!=1280 || b.height!=720 || !b.den || !b.num || b.num>120000 || b.den>10000 ||
        double(b.num)/b.den>60.01 || b.count<1 || b.count>1200 || b.audioSize>4u*1024u*1024u ||
        b.durationUs<1 || b.durationUs>20000000 || std::abs(b.duration()-double(b.count)*b.den/b.num)>0.1)
        throw std::runtime_error("Unsupported startup bank");
    b.audioOffset=40+(b.count+1)*4;
    if (std::uint64_t(b.audioOffset)+b.audioSize>=fileSize ||
        std::uint64_t(b.audioOffset)+b.audioSize>bytes.size() || u32(36)!=b.audioOffset+b.audioSize)
        throw std::runtime_error("Invalid startup table");
    for (unsigned i=0;i<=b.count;++i) b.offsets.push_back(u32(40+i*4));
    if (b.offsets.front()!=b.audioOffset+b.audioSize || b.offsets.back()!=fileSize)
        throw std::runtime_error("Invalid startup bounds");
    for (unsigned i=0;i<b.count;++i)
        if (b.offsets[i+1]<=b.offsets[i] || b.offsets[i+1]-b.offsets[i]>8u*1024u*1024u)
            throw std::runtime_error("Invalid startup frame");
    return b;
}
inline Bank parse(const std::vector<std::uint8_t>& bytes) { return parsePrefix(bytes,bytes.size()); }
// Only the table and PCM audio stay resident. One encoded frame is read per job.
struct Stream {
    std::ifstream file;
    std::vector<std::uint8_t> bytes;
    Bank bank;
    explicit Stream(const std::string& path):file(path,std::ios::binary|std::ios::ate) {
        const auto size=file.tellg();
        if (!file || size<40 || size>96*1024*1024) throw std::runtime_error("Startup clip is missing or incomplete");
        bytes.resize(40);file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(bytes.data()),40)) throw std::runtime_error("Startup header is incomplete");
        const std::uint32_t prefix=std::uint32_t(bytes[36])|std::uint32_t(bytes[37])<<8|
            std::uint32_t(bytes[38])<<16|std::uint32_t(bytes[39])<<24;
        if (prefix<40 || prefix>5u*1024u*1024u || prefix>=size) throw std::runtime_error("Invalid startup header size");
        bytes.resize(prefix);
        if (!file.read(reinterpret_cast<char*>(bytes.data()+40),prefix-40)) throw std::runtime_error("Startup audio is incomplete");
        bank=parsePrefix(bytes,static_cast<std::uint64_t>(size));
    }
    std::vector<std::uint8_t> frame(unsigned index) {
        if (index>=bank.count) throw std::runtime_error("Invalid startup frame index");
        std::vector<std::uint8_t> encoded(bank.offsets[index+1]-bank.offsets[index]);
        file.clear();file.seekg(bank.offsets[index]);
        if (!file.read(reinterpret_cast<char*>(encoded.data()),encoded.size())) throw std::runtime_error("Could not read startup frame");
        return encoded;
    }
};
} // namespace cards::boot360
