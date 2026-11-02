#pragma once
#include "core.hpp"
#include <QMainWindow>
#include <QFutureWatcher>
#include <atomic>
#include <memory>
class QGraphicsScene; class QGraphicsView; class QSlider; class QTableWidget;
class QListWidget;
class QDoubleSpinBox; class QLabel; class QPushButton; class QTimer; class QComboBox;
namespace orbitwatch {
class Window : public QMainWindow {
public:
    Window();
    ~Window() override;
private:
    std::shared_ptr<const Scenario> scenario_;
    Result result_;
    QString source_;
    bool has_result_=false;
    std::shared_ptr<std::atomic_bool> cancel_;
    QFutureWatcher<Result> watcher_;
    QGraphicsScene* scene_; QGraphicsView* view_; QSlider* time_;
    QTableWidget* table_; QDoubleSpinBox* threshold_; QLabel* status_;
    QPushButton* analyze_; QPushButton* open_; QPushButton* demo_; QPushButton* export_;
    QTimer* timer_; QComboBox* projection_;
    std::array<double,3> extents_{1,1,1};
    QListWidget* objects_;
    QLabel* objectsTitle_; QLabel* eventsTitle_; QLabel* details_; QLabel* clock_;
    QPushButton* demo100_; QPushButton* exportCsv_;
    int selected_=-1;
    void setScenario(Scenario s,QString source={});
    void render(); void run(); void exportReport(const QString& format); void updateDetails(); void showError(const QString& message);
};
}
