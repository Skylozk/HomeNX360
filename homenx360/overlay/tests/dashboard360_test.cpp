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
