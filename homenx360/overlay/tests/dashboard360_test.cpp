#include "../projects/cards/src/Dashboard360.hpp"
#include <cassert>
#include <queue>
#include <iostream>
using namespace cards::dash360;
int main() {
  // Every action must be reachable with a controller from the launch tile.
  for(bool isHome : {true,false}) for(int n=1;n<=(isHome?9:10);++n) {
    std::array<bool,10> seen{};std::queue<int> q;q.push(0);seen[0]=true;
    while(!q.empty()) {int i=q.front();q.pop();for(auto d : std::array<std::array<int,2>,4>{{{-1,0},{1,0},{0,-1},{0,1}}}) {
      int j=neighbor(i,n,isHome,d[0],d[1]);assert(j>=0&&j<n);
      if(!seen[j]){seen[j]=true;q.push(j);}
    }}
    for(int i=0;i<n;++i){assert(seen[i]);auto t=tile(i,isHome);assert(t.contains(t.x+t.w/2,t.y+t.h/2));
      assert(!t.contains(t.x+t.w,t.y));
      for(int j=i+1;j<n;++j){auto u=tile(j,isHome);assert(!(t.x<u.x+u.w&&u.x<t.x+t.w&&t.y<u.y+u.h&&u.y<t.y+t.h));}
    }
  }
  assert(neighbor(0,9,true,0,1)==1);assert(neighbor(1,9,true,0,1)==2);
  assert(neighbor(0,9,true,1,0)==3);assert(neighbor(3,9,true,0,1)==6);
  assert(wrapTab(-1)==7&&wrapTab(8)==0);
  std::cout<<"Dashboard navigation, tile hit areas and tab wrapping passed\n";
}

// NXE's single row must reach every game, including beyond the old ten-item page.
namespace {
struct NxeNavigationChecks {
  NxeNavigationChecks() {
    using namespace cards::nxe;
    for (int count : {0,1,4,9,10,11,100}) {
      int selected=0;
      for (int i=1;i<count;++i) { selected=nextCard(selected,1,count); assert(selected==i); }
      assert(nextCard(selected,1,count)==std::max(0,count-1));
      for (int i=count-2;i>=0;--i) { selected=nextCard(selected,-1,count); assert(selected==i); }
      assert(nextCard(0,-1,count)==0);
    }
    // Touch uses the same projected trapezoids as drawing, also while scrolling.
    for (int frame=0;frame<=100;++frame) {
      const float scroll=frame/100.f;
      for (int index=-2;index<=6;++index) {
        const auto p=panel(index-scroll);
        assert(p.w>0 && p.height(0)>0 && p.height(1)>0);
        assert(p.contains(p.x+p.w/2,p.top(0.5f)+p.height(0.5f)/2));
        assert(!p.contains(p.x+p.w,p.y+p.h/2));
      }
    }
    assert(ease(0)==0 && ease(1)==1);
    for (int i=0;i<100;++i) assert(ease(i/100.f)<=ease((i+1)/100.f));
    std::cout << "NXE full-library navigation, projected touch targets and transition easing passed\n";
  }
} nxeNavigationChecks;
}
