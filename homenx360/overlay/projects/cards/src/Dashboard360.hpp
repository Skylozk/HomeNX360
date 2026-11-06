#pragma once
#include <array>
#include "NxeLayout.hpp"
#include <cmath>
#include <limits>

// Logical 1280x720 coordinates, shared by drawing, touch and D-pad navigation.
namespace cards::dash360 {
struct Tile {
    float x, y, w, h;
    bool contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};
inline constexpr std::array<Tile, 9> home = {{
    {228,184,174,130}, {228,318,174,130}, {228,452,174,130},
    {406,184,462,264}, {872,184,178,64}, {872,252,178,196},
    {406,452,229,130}, {639,452,229,130}, {872,452,178,130}
}};
inline constexpr std::array<const char*, 8> tabs = {
    "bing", "home", "social", "games", "tv & movies", "music", "apps", "settings"
};
inline constexpr std::array<float, 8> tabX = {154,254,374,494,668,862,980,1090};
inline Tile grid(int index) {
    return {228.f + (index % 5) * 166.f, 184.f + (index / 5) * 202.f, 162.f, 198.f};
}
inline Tile tile(int index, bool isHome) { return isHome ? home.at(index) : grid(index); }
// Prefer a tile whose edge overlaps the current tile on the perpendicular
// axis. This makes the large hero panel behave naturally beside three rows.
inline int neighbor(int from, int count, bool isHome, int dx, int dy) {
    const auto a = tile(from, isHome);
    float best = std::numeric_limits<float>::max();
    int result = from;
    for (int i = 0; i < count; ++i) {
        if (i == from) continue;
        const auto b = tile(i, isHome);
        const float cx = b.x + b.w/2 - a.x - a.w/2;
        const float cy = b.y + b.h/2 - a.y - a.h/2;
        if ((dx && cx * dx <= 1) || (dy && cy * dy <= 1)) continue;
        const bool overlap = dx ? b.y < a.y+a.h && a.y < b.y+b.h
                                : b.x < a.x+a.w && a.x < b.x+b.w;
        const float score = (overlap ? 0.f : 10000.f) + (dx ? std::abs(cx) + std::abs(cy)*2 : std::abs(cy) + std::abs(cx)*2);
        if (score < best) { best = score; result = i; }
    }
    return result;
}
inline int wrapTab(int tab) { return (tab % 8 + 8) % 8; }
} // namespace cards::dash360
