#include "orbitwatch/earth_view.hpp"
#include <QLabel>
#include <QQuickWidget>
#include <QQuickWindow>
#include <QQmlError>
#include <QVBoxLayout>

static void initializeEarthResources() { Q_INIT_RESOURCE(earth); }

namespace orbitwatch {
EarthView::EarthView(QWidget* parent) : QWidget(parent) {
    initializeEarthResources();
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* view = new QQuickWidget(this);
    view->setObjectName("earthQuickView");
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    view->setClearColor(QColor("#050b16"));
    auto* error = new QLabel(this);
    error->setObjectName("earthErrorLabel");
    error->setWordWrap(true);
    error->setStyleSheet("color: #ffd5be; background: #231820; padding: 16px;");
    error->hide();
    connect(view, &QQuickWidget::statusChanged, this, [view, error](QQuickWidget::Status status) {
        if (status != QQuickWidget::Error) return;
        QStringList messages;
        for (const auto& entry : view->errors()) messages.append(entry.toString());
        error->setText("Earth view could not be loaded. The 2D analysis remains available.\n" + messages.join('\n'));
        error->show();
    });
    connect(view, &QQuickWidget::sceneGraphError, this, [error](QQuickWindow::SceneGraphError, const QString& message) {
        error->setText("3D graphics initialization failed. Use the 2D analysis tab.\n" + message);
        error->show();
    });
    layout->addWidget(view, 1);
    layout->addWidget(error);
    view->setSource(QUrl("qrc:/earth/qml/Earth.qml"));
}
}
