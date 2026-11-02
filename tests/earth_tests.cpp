#include "orbitwatch/earth_view.hpp"
#include "orbitwatch/window.hpp"
#include <memory>
#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QQuickItem>
#include <QQuickWidget>
#include <QTest>
#include <QPushButton>
#include <QTableWidget>
#include <QElapsedTimer>
#include <QWheelEvent>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
void wheel(QQuickWidget& view, int delta) {
    const QPoint center(view.width()/2, view.height()/2);
    QWheelEvent event(center, view.mapToGlobal(center), QPoint(), QPoint(0,delta),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(&view, &event);
}
}
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    const bool renderCheck = app.arguments().contains("--render-check");
    try {
        std::unique_ptr<QWidget> host;
        if(app.arguments().contains("--integrated")) host = std::make_unique<orbitwatch::Window>();
        else host = std::make_unique<orbitwatch::EarthView>();
        auto& earth = *host;
        const bool withAnalysis=app.arguments().contains("--with-analysis");
        earth.resize(withAnalysis?1500:1100, withAnalysis?900:650);
        earth.show();
        auto* view = earth.findChild<QQuickWidget*>("earthQuickView");
        require(view != nullptr, "Missing Quick widget");
        QTest::qWait(700);
        require(view->status() == QQuickWidget::Ready, "Earth QML failed to load");
        auto* root = view->rootObject();
        require(root != nullptr && root->findChild<QObject*>("earthGlobe"), "Missing globe model");
        auto* error = earth.findChild<QLabel*>("earthErrorLabel");
        require(error && error->isHidden(), "Graphics initialization failed");
        const auto property = [root](const char* name) { return root->property(name).toDouble(); };
        if(withAnalysis) {
            auto* demo=earth.findChild<QPushButton*>("demoButton");
            auto* analyze=earth.findChild<QPushButton*>("analyzeButton");
            auto* table=earth.findChild<QTableWidget*>("eventsTable");
            require(demo && analyze && table,"Integrated workspace missing controls");
            demo->click(); analyze->click();
            QElapsedTimer timeout; timeout.start();
            while(!analyze->isEnabled() && timeout.elapsed()<10000) QTest::qWait(20);
            require(analyze->isEnabled() && table->rowCount()>0,"Integrated analysis failed");
            QTest::qWait(100);
        }
        const auto initial = view->grabFramebuffer();
        if (renderCheck) {
            require(!initial.isNull(), "Empty framebuffer");
            int surfacePixels = 0;
            for (int y=initial.height()/3; y<initial.height()*2/3; ++y)
                for (int x=initial.width()/3; x<initial.width()*2/3; ++x) {
                    const auto color=initial.pixelColor(x,y);
                    if(color.blue()>45 && color.green()>35) ++surfacePixels;
                }
            require(surfacePixels>3000, "Earth surface not rendered in framebuffer");
            const auto path=qEnvironmentVariable("ORBITWATCH_SCREENSHOT");
            if(!path.isEmpty()) require(earth.grab().save(path), "Could not save preview");
        }
        const QPoint center(view->width()/2, view->height()/2);
        QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, center);
        QTest::mouseMove(view, center + QPoint(90,35), 30);
        QTest::mouseRelease(view, Qt::LeftButton, Qt::NoModifier, center + QPoint(90,35));
        QTest::qWait(100);
        require(std::abs(property("yaw")) > 1, "Mouse drag did not rotate camera");
        if(renderCheck) require(view->grabFramebuffer()!=initial, "Rotation did not change rendered image");
        const auto before = property("cameraDistance");
        wheel(*view, 120);
        require(property("cameraDistance") < before, "Wheel did not zoom in");
        wheel(*view, -120);
        require(std::abs(property("cameraDistance")-before)<0.001, "Wheel did not zoom out");
        for (int i=0; i<100; ++i) wheel(*view, 120);
        require(property("cameraDistance") == property("minimumDistance"), "Near zoom limit failed");
        for (int i=0; i<100; ++i) wheel(*view, -120);
        require(property("cameraDistance") == property("maximumDistance"), "Far zoom limit failed");
        QTest::mouseDClick(view, Qt::LeftButton, Qt::NoModifier, center);
        require(property("yaw")==0 && property("pitch")==-12 && property("cameraDistance")==315,
                "Double-click did not reset camera");
        std::cout << "Earth load, drag, zoom limits and reset passed";
        if(renderCheck) std::cout << "; rendered surface verified";
        std::cout << '\n';
        return 0;
    } catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
