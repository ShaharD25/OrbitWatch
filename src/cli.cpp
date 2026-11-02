#include "orbitwatch/io.hpp"
#include <QCoreApplication>
#include <iostream>
int main(int argc,char** argv) {
    QCoreApplication app(argc,argv); auto args=app.arguments();
    try {
        if(args.size()==5 && args[1]=="--generate") {
            bool ok=false; auto count=args[2].toUInt(&ok); bool valid=false; auto samples=args[3].toUInt(&valid);
            if(!ok || !valid) throw std::runtime_error("Invalid demo size");
            orbitwatch::write_scenario(orbitwatch::make_demo(count,samples),args[4]); return 0;
        }
        if(args.size()!=4) { std::cerr<<"Usage: orbitwatch_cli INPUT.json THRESHOLD_M OUTPUT.json|csv\n       orbitwatch_cli --generate OBJECTS SAMPLES OUTPUT.json\n"; return 2; }
        bool ok=false; double threshold=args[2].toDouble(&ok); if(!ok) throw std::runtime_error("Invalid threshold");
        auto s=orbitwatch::read_scenario(args[1]); auto r=orbitwatch::analyze(s,threshold);
        orbitwatch::write_report(s,r,args[1],args[3]); std::cout<<r.events.size()<<" events\n"; return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
