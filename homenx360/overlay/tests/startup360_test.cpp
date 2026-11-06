#include "../projects/cards/src/BootFormat.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
int main(int argc,char** argv) {
    assert(argc==2);
    cards::boot360::SkipGate gate;
    assert(!gate.consume(true,true)); // A inherited from launching the app
    assert(!gate.consume(true,false));
    assert(!gate.consume(false,false));
    assert(gate.consume(true,true)); // a new deliberate press can skip
    gate={};
    assert(!gate.consume(false,false));
    assert(gate.consume(true,true));
    std::ifstream f(argv[1],std::ios::binary);
    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(f)),{});
    const auto bank=cards::boot360::parse(bytes);
    cards::boot360::Stream stream(argv[1]);
    assert(stream.bytes.size()<2u*1024u*1024u); // regression: never preload the 55 MB movie
    assert(stream.bank.offsets==bank.offsets);
    size_t largest=0;
    for(unsigned i=0;i<bank.count;++i) {
        const auto encoded=stream.frame(i);
        largest=std::max(largest,encoded.size());
        assert(std::equal(encoded.begin(),encoded.end(),bytes.begin()+bank.offsets[i]));
    }
    assert(largest<1024u*1024u);
    std::cout<<"Streaming prefix: "<<stream.bytes.size()<<" bytes; largest frame: "<<largest<<" bytes\n";
    assert(bank.count==288 && bank.num==30 && bank.den==1);
    assert(bank.frameAt(0)==0 && bank.frameAt(1)==30 && bank.frameAt(99)==287);
    assert(bank.frameAt(-1)==0);
    assert(bank.width==1280 && bank.height==720);
    assert(bank.duration()>9.60 && bank.duration()<9.63);
    auto rejects=[](const auto& data) { try { (void)cards::boot360::parse(data);return false; } catch (...) { return true; } };
    auto bad=bytes;bad[0]=0;assert(rejects(bad));
    bad=bytes;bad[20]=bad[21]=bad[22]=bad[23]=0;assert(rejects(bad));
    bad=bytes;bad[24]=255;bad[25]=255;assert(rejects(bad));
    bad=bytes;bad[40]=0;bad[41]=0;bad[42]=0;bad[43]=0;assert(rejects(bad));
    bad=bytes;bad.pop_back();assert(rejects(bad));
    assert(rejects(std::vector<std::uint8_t>(bytes.begin(),bytes.begin()+39)));
    std::cout << "Startup bank, 288-frame timing, audio bounds and malformed-file checks passed\n";
}
