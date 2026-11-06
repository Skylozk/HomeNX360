#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace cards::boot360 {
struct Bank {
    std::uint32_t width=0,height=0,num=0,den=0,count=0,durationUs=0,audioSize=0,audioOffset=0;
    std::vector<std::uint32_t> offsets;
    unsigned frameAt(double seconds) const {
        return static_cast<unsigned>(std::clamp(std::floor(seconds*num/den),0.,double(count-1)));
    }
    double duration() const { return durationUs/1000000.; }
};
inline Bank parse(const std::vector<std::uint8_t>& bytes) {
    const char magic[8]={'N','X','B','O','O','T','1','\0'};
    if (bytes.size()<40 || bytes.size()>96u*1024u*1024u || !std::equal(magic,magic+8,bytes.begin()))
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
    if (std::uint64_t(b.audioOffset)+b.audioSize>=bytes.size() || u32(36)!=b.audioOffset+b.audioSize)
        throw std::runtime_error("Invalid startup table");
    for (unsigned i=0;i<=b.count;++i) b.offsets.push_back(u32(40+i*4));
    if (b.offsets.front()!=b.audioOffset+b.audioSize || b.offsets.back()!=bytes.size())
        throw std::runtime_error("Invalid startup bounds");
    for (unsigned i=0;i<b.count;++i)
        if (b.offsets[i+1]<=b.offsets[i] || b.offsets[i+1]-b.offsets[i]>8u*1024u*1024u)
            throw std::runtime_error("Invalid startup frame");
    return b;
}
} // namespace cards::boot360
