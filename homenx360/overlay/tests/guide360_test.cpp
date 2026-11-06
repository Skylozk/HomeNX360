#include "../projects/cards/src/GuideLayout.hpp"
#include <cassert>
#include <iostream>
int main() {
    using namespace cards::guide360;
    assert(wrap(-1)==4 && wrap(5)==0);
    for(int selected=0;selected<5;++selected) {
        const auto body=blade(selected,selected);
        assert(body.y==200 && body.h==316);
        for(int i=0;i<5;++i) {
            const auto box=blade(i,selected);
            assert(box.x>=286 && box.x+box.w<=986);
            if(i!=selected) assert(hitBlade(box.x+box.w/2,box.y+box.h/2,selected)==i);
            for(int j=i+1;j<5;++j) {const auto other=blade(j,selected);assert(box.x+box.w<=other.x);}
        }
        for(int count=1;count<=7;++count) for(int i=0;i<count;++i) {
            const auto box=row(i,selected);
            assert(box.x>=body.x && box.x+box.w<=body.x+body.w);
            assert(box.y+box.h<=body.y+body.h);
            assert(hitRow(box.x+box.w/2,box.y+box.h/2,selected,count)==i);
            assert(!box.contains(box.x+box.w,box.y));
        }
    }
    std::cout << "Guide blade bounds, seven-row layouts, touch targets and tab wrapping passed\n";
}
