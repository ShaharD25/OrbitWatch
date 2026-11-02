#include "orbitwatch/io.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <iostream>
int main(int argc,char** argv) {
    QCoreApplication app(argc,argv);
    try {
        QTemporaryDir dir; if(!dir.isValid()) throw std::runtime_error("Temporary directory failed");
        auto path=dir.filePath("scenario.json"); auto s=orbitwatch::make_demo(100,20);
        orbitwatch::write_scenario(s,path); auto loaded=orbitwatch::read_scenario(path);
        if(loaded.objects.size()!=100 || loaded.objects[5].positions!=s.objects[5].positions || loaded.times_us!=s.times_us) throw std::runtime_error("Round-trip mismatch");
        auto r=orbitwatch::analyze(loaded,10000); orbitwatch::write_report(loaded,r,path,dir.filePath("report.json")); orbitwatch::write_report(loaded,r,path,dir.filePath("report.csv"));
        QFile f(path); if(!f.open(QIODevice::WriteOnly) || f.write("{broken")!=7) throw std::runtime_error("Could not create malformed fixture"); f.close();
        bool rejected=false; try { orbitwatch::read_scenario(path); } catch(const std::exception&) { rejected=true; }
        if(!rejected) throw std::runtime_error("Malformed JSON accepted");
        std::cout<<"I/O checks passed\n"; return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
