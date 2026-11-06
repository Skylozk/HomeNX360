#pragma once
#include "SettingsLayout.hpp"
#include <cmath>

// Shared drawing primitives for the native renderer and the host visual preview.
namespace cards::settings360 {
struct Color { float r,g,b,a=1; };
template<class Paint> void background(Paint& p,float opacity) {
    for(int x=0;x<1280;x+=8) {
        const float u=(x+4)/1280.f;
        const float light=std::exp(-std::pow((u-0.40f)/0.32f,2.f));
        const Color top{0.10f+0.23f*light,0.17f+0.32f*light,0.025f+0.15f*u,opacity};
        const Color middle{0.35f+0.25f*light+0.02f*u,0.54f+0.18f*light-0.10f*u,0.10f+0.38f*u,opacity};
        const Color horizon{0.79f+0.16f*u,0.86f-0.04f*u,0.88f-0.36f*u,opacity};
        p.gradient({float(x),0,8,270},top,middle);
        p.gradient({float(x),270,8,168},middle,horizon);
        p.gradient({float(x),438,8,282},horizon,{0.59f,0.61f,0.60f,opacity});
    }
    // Low contrast concentric circles, as in the original NXE settings theme.
    for(int i=0;i<42;++i) {
        const float cx=float((i*317+13)%1320)-20,cy=float((i*193+23)%760)-20;
        const float radius=9.f+(i*11)%38;
        for(int ring=0;ring<3;++ring) for(int segment=0;segment<24;++segment) {
            const float a=segment*6.2831853f/24,b=(segment+1)*6.2831853f/24;
            const float size=radius+ring*5.f;
            p.line(cx+std::cos(a)*size,cy+std::sin(a)*size,
                   cx+std::cos(b)*size,cy+std::sin(b)*size,{0.95f,1,0.89f,0.032f*opacity},1.4f);
        }
    }
}
template<class Paint> void frame(Paint& p,float opacity) {
    p.gradient({194,596,890,74},{0.24f,0.29f,0.30f,0.18f*opacity},{0.24f,0.29f,0.30f,0.f});
    p.rounded({194,113,890,484},{0.1f,0.15f,0.17f,0.16f*opacity},4);
    p.rounded(panel,{0.18f,0.24f,0.28f,opacity},4);
    p.gradient({196,112,886,480},{0.30f,0.36f,0.39f,opacity},{0.18f,0.23f,0.27f,opacity});
}
template<class Paint> void selection(Paint& p,Rect r,float opacity) {
    p.rounded(r,{0.46f,0.63f,0.11f,opacity},4);
    p.gradient({r.x+1,r.y+1,r.w-2,r.h-2},{0.67f,0.80f,0.36f,opacity},{0.36f,0.62f,0.015f,opacity});
    p.gradient({r.x+1,r.y+1,r.w-2,r.h*0.47f},{0.94f,0.97f,0.83f,0.62f*opacity},{0.7f,0.85f,0.38f,0.f});
}
template<class Paint> void orb(Paint& p,float opacity) {
    constexpr float cx=1157,cy=635,radius=25;
    for(int ring=0;ring<4;++ring) for(int segment=0;segment<40;++segment) {
        const float a=segment*6.2831853f/40,b=(segment+1)*6.2831853f/40;
        const float size=20.f+ring*15;
        p.line(cx+std::cos(a)*size,674+std::sin(a)*size*0.23f,
               cx+std::cos(b)*size,674+std::sin(b)*size*0.23f,{0.83f,0.85f,0.79f,0.18f*opacity},1);
    }
    p.circle(cx,cy+3,radius,{0.19f,0.23f,0.17f,0.4f*opacity});
    p.circle(cx,cy,radius,{0.54f,0.57f,0.51f,opacity});
    for(int y=-24;y<25;y+=2) {
        const float width=std::sqrt(std::max(0.f,radius*radius-float(y*y)));
        const float shade=0.51f+0.40f*std::exp(-std::pow((y+10)/20.f,2.f));
        p.rect({cx-width,cy+y,width*2,2},{shade,shade+0.01f,shade-0.02f,opacity});
    }
    // Curved vector adaptation of the green X on the silver sphere.
    for(int y=-21;y<21;y+=2) {
        const float at=16.f*std::sqrt(std::abs(y)/21.f),width=3.5f+2*(1-std::abs(y)/21.f);
        for(int side:{-1,1}) {
            p.rect({cx+side*at-width/2,cy+y,width,2},{0.28f,0.55f,0.015f,opacity});
            p.rect({cx+side*at-width/2,cy+y,1,2},{0.63f,0.85f,0.16f,opacity});
        }
    }
}
template<class Paint> void systemIcon(Paint& p,int index,float x,float y,float opacity) {
    const Color silver{0.77f,0.83f,0.83f,opacity},light{0.89f,0.94f,0.92f,opacity},dark{0.42f,0.50f,0.52f,opacity};
    if(index==0 || index==6) {
        p.rect({x-7,y-16,13,31},silver);p.rect({x-7,y-16,4,31},light);
        p.circle(x,y+2,2.5f,{0.39f,0.59f,0.16f,opacity});
        p.line(x-6,y-13,x+5,y-14,light,2);
        if(index==6){p.rounded({x+2,y+4,18,10},light,3);p.line(x+5,y+9,x+10,y+9,dark,2);}
    } else if(index==1) {
        p.circle(x-7,y-11,5,silver);p.line(x-7,y-3,x,y+8,silver,11);
        p.circle(x+8,y-4,4,light);p.line(x+8,y+3,x+9,y+10,light,7);
        p.rounded({x+7,y+7,10,10},dark,2);p.rect({x+9,y+9,6,6},light);
    } else if(index==2) {
        p.line(x-13,y+4,x+10,y-3,dark,15);p.line(x-13,y+1,x+10,y-6,silver,11);
        p.line(x-9,y-6,x+4,y-10,light,4);
    } else if(index==3) {
        p.line(x-9,y+8,x+4,y-8,dark,4);p.line(x+4,y-8,x+13,y+8,dark,4);
        p.circle(x-10,y+9,6,silver);p.circle(x+3,y-9,6,light);p.circle(x+14,y+5,6,silver);
    } else if(index==4) {
        p.rounded({x-15,y-13,29,22},silver,4);p.rounded({x-12,y-10,23,16},dark,2);
        p.rect({x-4,y+9,8,4},silver);p.line(x-10,y+15,x+9,y+15,silver,3);
    } else if(index==5) {
        p.rounded({x-10,y-12,20,21},silver,5);p.circle(x,y-3,6,dark);p.circle(x,y-4,4,light);
        p.rect({x-3,y+9,6,5},dark);p.line(x-11,y+16,x+11,y+16,silver,2);
    } else {
        for(int i=0;i<8;++i){float a=i*6.2831853f/8;p.line(x+std::cos(a)*8,y+std::sin(a)*8,x+std::cos(a)*14,y+std::sin(a)*14,silver,5);}
        p.circle(x,y,10,silver);p.circle(x,y,4,dark);
    }
}
} // namespace cards::settings360
