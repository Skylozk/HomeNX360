// Native settings navigation and rendering.
#include "SettingsBindings.inc"

#include "SettingsNavigation.inc"

void CardsActivity::drawSheet(nxui::Renderer& r) {
    if(!m_sheetOpen || m_sheet.empty()) return;
    using namespace settings360;
    struct Paint {
        nxui::Renderer& r;
        static nxui::Color color(Color c){return {c.r,c.g,c.b,c.a};}
        static nxui::Rect box(Rect b){return {b.x,b.y,b.w,b.h};}
        void rect(Rect b,Color c){r.drawRect(box(b),color(c));}
        void rounded(Rect b,Color c,float radius){r.drawRoundedRect(box(b),color(c),radius);}
        void gradient(Rect b,Color t,Color d){r.drawGradientRect(box(b),color(t),color(d));}
        void line(float x,float y,float xx,float yy,Color c,float w){r.drawLine({x,y},{xx,yy},color(c),w);}
        void circle(float x,float y,float radius,Color c){r.drawCircle({x,y},radius,color(c),40);}
    } paint{r};
    const float a=nxe::ease(m_sheetIn),transition=nxe::ease(m_sheetRowsIn);
    const float slide=24*m_sheetDirection*(1-transition);
    const auto white=nxui::Color(0.93f,0.95f,0.96f,a);
    const auto muted=nxui::Color(0.70f,0.74f,0.76f,a);
    background(paint,a,m_settings.accent);frame(paint,a);orb(paint,a,m_settings.accent);
    textMid(r,m_fGuide,titles[m_sheetSec],96,68,{0.1f,0.14f,0.10f,0.35f*a},1.3f);
    textMid(r,m_fGuide,titles[m_sheetSec],95,67,{0.88f,0.92f,0.80f,a},1.3f);
    auto& rows=m_sheet[m_sheetSec].rows;
    const int count=int(rows.size());
    m_sheetRow=std::clamp(m_sheetRow,0,std::max(0,count-1));
    r.pushClipRect({rowX-1,rowY,rowW+2,listBottom-rowY});
    if(count) selection(paint,row(0,m_sheetScroll-m_sheetFocus*pitch,slide),a*transition,m_settings.accent);
    for(int i=0;i<count;++i) {
        const auto b=row(i,m_sheetScroll,slide);
        if(b.y+b.h<=rowY || b.y>=listBottom) continue;
        const bool selected=i==m_sheetRow;
        if(i+1<count && !selected && i+1!=m_sheetRow)
            r.drawRect({b.x+4,b.y+b.h-1,b.w-8,1.5f},{0.65f,0.72f,0.75f,0.25f*a*transition});
        float textX=b.x+13;
        if(m_sheetSec==System) {
            systemIcon(paint,i,b.x+26,b.y+24,a*transition);textX=b.x+54;
        } else if(rows[i].glyph) {
            icon(r,rows[i].glyph,b.x+26,b.y+24,29,white.withAlpha(a*transition));textX=b.x+54;
        }
        textMid(r,m_fGuide,fit(m_fGuide,rows[i].label,b.x+b.w-textX-10,1),textX,b.y+24,
                (selected?white:muted).withAlpha(a*transition),1);
    }
    r.popClipRect();
    if(count) {
        const auto& item=rows[m_sheetRow];
        float y=147;
        auto paragraph=[&](const std::string& text,nxui::Color color,float spacing=30.f) {
            for(const auto& line:wrap(m_fGuide,text,380)) {
                if(y>556) break;
                textMid(r,m_fGuide,line,665,y,color,1);y+=spacing;
            }
        };
        const std::string value=item.kind==MenuItem::Toggle?(item.level && item.level()>0.5f?"On":"Off"):
            item.value?item.value():std::string();
        if(!value.empty()) {paragraph("Current Setting",white);paragraph(value,white);y+=35;}
        if(item.kind==MenuItem::Toggle || item.kind==MenuItem::Choice || item.kind==MenuItem::Slider) {
            // Fixed positions are shared with the touch controls above.
            y=255;textMid(r,m_fGuide,"<",678,y,white);textMid(r,m_fGuide,">",1025,y,white);
            if(item.kind==MenuItem::Slider) {
                const float level=item.level?clamp01(item.level()):0.f;
                r.drawRect({720,252,260,6},{0.75f,0.80f,0.79f,0.35f*a});
                r.drawRect({720,252,260*level,6},nxeColor({0.61f,0.80f,0.18f,a}));
                r.drawRect({715+260*level,244,10,22},white);
            } else textMid(r,m_fBody,"Left / Right to change",738,y,muted,0.9f);
            y=310;
        }
        if(!item.detail.empty()) paragraph(item.detail,white);
        else if(item.kind==MenuItem::Info) paragraph("Information reported by this Switch and HomeNX.",muted);
        if(item.kind==MenuItem::Toggle || item.kind==MenuItem::Choice || item.kind==MenuItem::Slider) {
            y=542;paragraph("Changes are applied immediately.",muted);
        }
    }
    if(count>visibleRows) {
        const float h=460.f*visibleRows/count;
        r.drawRect({639,125,3,460},{0.4f,0.47f,0.48f,0.4f*a});
        r.drawRect({639,125+(460-h)*m_sheetRow/std::max(1,count-1),3,h},{0.76f,0.82f,0.76f,0.8f*a});
    }
    const char* letters[]={"A","B"};const char* labels[]={"Select","Back"};
    for(int i=0;i<2;++i) {
        const float x=111+i*110.f,y=650;
        paint.circle(x,y+2,12,{0.14f,0.16f,0.13f,0.35f*a});
        paint.circle(x,y,11,i==0?Color{0.39f,0.63f,0.08f,a}:Color{0.65f,0.12f,0.13f,a});
        paint.circle(x-2,y-3,7,{0.94f,0.96f,0.78f,0.13f*a});
        textMid(r,m_fBadge,letters[i],x-5,y,{0.12f,0.16f,0.10f,a},1);
        textMid(r,m_fBody,labels[i],x+20,y,{0.97f,0.97f,0.94f,a},0.95f);
    }
}
