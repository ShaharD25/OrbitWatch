#include "orbitwatch/window.hpp"
#include <QApplication>
#include <QTimer>
int main(int argc,char** argv) {
    QApplication app(argc,argv);
    orbitwatch::Window window; window.show();
    if(app.arguments().contains("--smoke-test")) QTimer::singleShot(500,&app,&QApplication::quit);
    return app.exec();
}
