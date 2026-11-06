#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace cards::nxe {
inline constexpr std::array<const char*,8> channels = {
    "Welcome", "My Xbox", "Friends", "Game Library", "Video Library", "Music Library", "Homebrew", "System Settings"
};
inline int nextCard(int selected, int delta, int count) {
    return count > 0 ? std::clamp(selected + delta, 0, count - 1) : 0;
}
inline float ease(float t) { t=std::clamp(t,0.f,1.f); return 1.f-(1.f-t)*(1.f-t)*(1.f-t); }
struct Panel {
    float x,y,w,h,skew;
    float top(float u) const { return y+skew*u; }
    float height(float u) const { return h-2.f*skew*u; }
    bool contains(float px,float py) const {
        if (px<x || px>=x+w) return false;
        const float u=(px-x)/w;
        return u>=0.f && u<1.f && py>=top(u) && py<top(u)+height(u);
    }
};
// One front card, then a receding stack. Fractional offsets animate continuously.
inline Panel panel(float relative, float rowOffset=0.f) {
    const float distance=std::abs(relative);
    const float focus=std::max(0.f,1.f-distance);
    const float w=156.f+118.f*focus;
    const float h=262.f+72.f*focus;
    const float x=relative>=0.f ? 300.f+274.f*std::min(relative,1.f)+112.f*std::max(0.f,relative-1.f)
                             : 300.f+160.f*relative;
    return {x,260.f+(334.f-h)/2.f+rowOffset,w,h,relative>0.f ? 20.f*(1.f-focus):0.f};
}
} // namespace cards::nxe
