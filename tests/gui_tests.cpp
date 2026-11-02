#include "orbitwatch/window.hpp"
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QGraphicsView>
#include <QPixmap>
#include <algorithm>
#include <vector>
#include <QPushButton>
#include <QSlider>
#include <QTableWidget>
#include <QTimer>
#include <QTabWidget>
#include <iostream>
#include <stdexcept>

template<class T> T* widget(QWidget& window, const char* name) {
    auto* result=window.findChild<T*>(name);
    if(!result) throw std::runtime_error(std::string("Missing widget: ")+name);
    return result;
}
void require(bool condition,const char* description) {
    if(!condition) throw std::runtime_error(description);
}
int main(int argc,char** argv) {
    QApplication app(argc,argv);
    orbitwatch::Window window;
    window.show();
    auto* tabs=widget<QTabWidget>(window,"viewTabs");
    tabs->setCurrentIndex(tabs->count()-1);
    QElapsedTimer elapsed;
    elapsed.start();
    int stage=0;
    QTimer poll;
    QObject::connect(&poll,&QTimer::timeout,&app,[&] {
        try {
            if(elapsed.elapsed()>20000) throw std::runtime_error("GUI analysis timeout");
            auto* analyze=widget<QPushButton>(window,"analyzeButton");
            auto* table=widget<QTableWidget>(window,"eventsTable");
            auto* exportButton=widget<QPushButton>(window,"exportButton");
            auto* slider=widget<QSlider>(window,"timeSlider");
            if(stage==0) {
                widget<QPushButton>(window,"demoButton")->click();
                require(slider->maximum()==999,"500-object demo should have 1000 samples");
                require(widget<QListWidget>(window,"objectsList")->count()==500,"Objects panel does not reflect loaded data");
                auto* search=widget<QLineEdit>(window,"objectSearch"); search->setText("OBJ-500");
                int visible=0; auto* objects=widget<QListWidget>(window,"objectsList");
                for(int i=0;i<objects->count();++i) if(!objects->item(i)->isHidden()) ++visible;
                require(visible==1,"Object search did not filter loaded data"); search->clear();
                analyze->click();
                require(!analyze->isEnabled(),"Analyze must be disabled during work");
                stage=1;
                return;
            }
            if(!analyze->isEnabled()) return;
            if(stage==2) {
                require(table->rowCount()==0 && !exportButton->isEnabled(),"Cancelled analysis published results");
                std::cout<<"GUI flow and cancellation passed\n";
                app.exit(0);
                return;
            }
            require(table->rowCount()>0 && exportButton->isEnabled(),"Analysis did not publish events");
            int row=-1;
            for(int i=0;i<table->rowCount();++i)
                if(table->item(i,0)->text()=="OBJ-1" && table->item(i,1)->text()=="OBJ-2") { row=i; break; }
            require(row>=0,"Known encounter missing from GUI");
            require(table->item(row,5)->text().toDouble()==100,"Known minimum should be 100 m");
            table->cellClicked(row,0);
            require(slider->value()==500,"Event selection did not navigate to minimum");
            table->selectRow(row);
            const auto detail=widget<QLabel>(window,"selectedEventDetails")->text();
            require(detail.contains("OBJ-1") && detail.contains("OBJ-2") && detail.contains("100.000 m"),"Event details do not match actual result");
            require(widget<QPushButton>(window,"exportCsvButton")->isEnabled(),"CSV export not enabled");
            widget<QPushButton>(window,"focusEventButton")->click();
            require(tabs->currentIndex()==tabs->count()-1,"Focus event must open 2D view");
            const auto screenshot=qEnvironmentVariable("ORBITWATCH_SCREENSHOT");
            if(!screenshot.isEmpty()) require(window.grab().save(screenshot),"Could not save screenshot");
            std::vector<double> frame_ms;
            auto* view=window.findChild<QGraphicsView*>();
            require(view!=nullptr,"Missing graphics view");
            for(int sample=501;sample<601;++sample) {
                QElapsedTimer frame; frame.start();
                slider->setValue(sample); view->viewport()->repaint();
                frame_ms.push_back(frame.nsecsElapsed()/1e6);
            }
            std::sort(frame_ms.begin(),frame_ms.end());
            std::cout<<"500 objects, selected paths: update+repaint median "<<frame_ms[50]<<" ms, p95 "<<frame_ms[95]<<" ms (not presentation FPS)\n";
            widget<QComboBox>(window,"projectionSelector")->setCurrentIndex(2);
            slider->setValue(501);
            widget<QDoubleSpinBox>(window,"thresholdInput")->setValue(20000);
            require(table->rowCount()==0 && !exportButton->isEnabled(),"Threshold change must invalidate results");
            require(!widget<QPushButton>(window,"exportCsvButton")->isEnabled() && !widget<QPushButton>(window,"focusEventButton")->isEnabled(),"Stale event actions remain enabled");
            analyze->click();
            widget<QPushButton>(window,"cancelButton")->click();
            stage=2;
        } catch(const std::exception& error) {
            std::cerr<<error.what()<<'\n';
            app.exit(1);
        }
    });
    poll.start(20);
    return app.exec();
}
