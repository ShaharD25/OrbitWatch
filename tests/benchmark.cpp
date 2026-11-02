#include "orbitwatch/core.hpp"
#include <chrono>
#include <iostream>
int main() {
    for(auto n:{100,250,500}) for(auto samples:{100,1000}) {
        auto s=orbitwatch::make_demo(n,samples);
        auto start=std::chrono::steady_clock::now(); auto result=orbitwatch::analyze(s,10000);
        auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
        std::cout<<n<<" objects, "<<samples<<" samples: "<<elapsed<<" s, "<<result.events.size()<<" events\n";
    }
}
