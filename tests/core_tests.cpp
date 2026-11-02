#include "orbitwatch/core.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
using namespace orbitwatch;
void check(bool value,const char* label) { if(!value) throw std::runtime_error(label); }
template<class F> void rejects(F f) { bool caught=false; try{f();}catch(const std::exception&){caught=true;} check(caught,"Expected rejection"); }
int main() { try {
    check(distance({0,0,0},{2,3,6})==7,"3D distance");
    Scenario s{"known",{0,1000000,2000000,3000000,4000000},{{"B",{{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},{"A",{{2,0,0},{1,0,0},{0,0,0},{2,0,0},{1,0,0}}}}};
    auto r=analyze(s,1); check(r.events.size()==2,"Split events");
    auto e=r.events[0]; check(e.first=="A" && e.second=="B" && e.start==1 && e.end==2 && e.minimum==2 && e.distance_m==0,"Known encounter");
    check(r.events[1].start==4 && r.events[1].end==4,"Single boundary sample");
    auto translated=s; for(auto& o:translated.objects) for(auto& p:o.positions) for(auto& v:p) v+=100;
    auto shifted=analyze(translated,1); check(shifted.events.size()==r.events.size() && shifted.events[0].distance_m==e.distance_m,"Translation invariant");
    std::swap(translated.objects[0],translated.objects[1]); check(analyze(translated,1).events[0].first=="A","Order invariant");
    auto bad=s; bad.objects[1].id="B"; rejects([&]{validate(bad);});
    bad=s; bad.times_us[1]=0; rejects([&]{validate(bad);});
    bad=s; bad.objects[0].positions.pop_back(); rejects([&]{validate(bad);});
    bad=s; bad.objects[0].positions[0][0]=std::numeric_limits<double>::infinity(); rejects([&]{validate(bad);});
    rejects([&]{analyze(s,0);}); rejects([&]{analyze(s,std::numeric_limits<double>::quiet_NaN());});
    rejects([&]{analyze(s,1,[]{return true;});}); rejects([&]{analyze(s,1,{},1);});
    Scenario crossing{"missed",{0,1000000},{{"A",{{-2,0,0},{2,0,0}}},{"B",{{0,0,0},{0,0,0}}}}};
    check(analyze(crossing,1).events.empty(),"Between-sample crossing not detected");
    crossing.objects[0].positions={{0,0,5},{0,0,5}}; check(analyze(crossing,1).events.empty(),"Projection overlap is not proximity");
    crossing.objects[0].positions={{1,0,0},{1,0,0}}; auto tie=analyze(crossing,1); check(tie.events.size()==1 && tie.events[0].minimum==0,"Inclusive threshold and earliest tie");
    // Independent reference using hypot for each sampled pair.
    std::mt19937 rng(42); std::uniform_real_distribution<double> random(-10,10);
    auto reference=make_demo(12,30);
    for(auto& o:reference.objects) for(auto& p:o.positions) for(auto& v:p) v=random(rng);
    auto actual=analyze(reference,8);
    for(std::size_t i=0;i<reference.objects.size();++i) for(std::size_t j=i+1;j<reference.objects.size();++j) {
        std::size_t count=0; bool previous=false;
        for(std::size_t k=0;k<reference.times_us.size();++k) {
            bool close=distance(reference.objects[i].positions[k],reference.objects[j].positions[k])<=8;
            if(close && !previous) ++count;
            previous=close;
        }
        std::size_t found=0;
        for(const auto& event:actual.events) {
            auto a=reference.objects[i].id,b=reference.objects[j].id; if(b<a) std::swap(a,b);
            if(event.first==a && event.second==b) ++found;
        }
        check(count==found,"Reference event counts");
    }
    for(auto n:{100,250,500}) { auto demo=make_demo(n); validate(demo); auto events=analyze(demo,1000); check(!events.events.empty(),"Demo encounter"); }
    std::cout<<"All core checks passed\n"; return 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; } }
