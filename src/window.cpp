#include "orbitwatch/window.hpp"
#include "orbitwatch/io.hpp"
#ifdef ORBITWATCH_HAS_3D
#include "orbitwatch/earth_view.hpp"
#endif
#include <QtWidgets>
#include <QtConcurrent/QtConcurrentRun>
#include <cmath>
#include <algorithm>
namespace orbitwatch {
Window::Window() {
    setWindowTitle("OrbitWatch — Satellite Proximity Analysis"); resize(1500,900);
    setMinimumSize(1050,650);
    setStyleSheet(R"(
        QMainWindow, QWidget#workspace { background: #070e18; color: #dce7f5; }
        QWidget { color: #dce7f5; font-size: 12px; }
        QLabel { background: transparent; border: none; }
        QLabel#brand { font-size: 23px; font-weight: 600; }
        QLabel#muted { color: #899db7; font-size: 11px; }
        QLabel#sectionTitle { font-size: 15px; font-weight: 600; padding-bottom: 7px; }
        QFrame#panel { background: #0d1726; border: 1px solid #20324a; border-radius: 8px; }
        QFrame#header { background: #0a1421; border-bottom: 1px solid #20324a; }
        QPushButton { background: #16263b; border: 1px solid #2a4160; border-radius: 5px;
                      padding: 8px 12px; text-align: left; }
        QPushButton:hover { background: #203955; border-color: #4786b9; }
        QPushButton:disabled { color: #52657c; border-color: #1a2a3e; background: #111e2f; }
        QPushButton#analyzeButton { background: #0968d8; border-color: #2789ed; font-weight: 600; }
        QPushButton#analyzeButton:hover { background: #1879e5; }
        QPushButton#analyzeButton:disabled { background: #16385a; color: #728aa2; }
        QLineEdit, QDoubleSpinBox, QComboBox { background: #0a1422; border: 1px solid #2b405a;
                      border-radius: 5px; padding: 7px; selection-background-color: #1767a6; }
        QTabWidget::pane { border: 1px solid #20324a; background: #030810; border-radius: 6px; }
        QTabBar::tab { background: #101d2f; color: #92a9c4; padding: 11px 24px; }
        QTabBar::tab:selected { background: #102d50; color: #66caff; border-bottom: 2px solid #159bf5; }
        QHeaderView::section { background: #101f32; color: #a5b8d0; padding: 8px 2px; border: none; font-size: 11px; }
        QTableWidget, QListWidget { background: #091320; border: 1px solid #1b2b40;
                      border-radius: 4px; gridline-color: #16273c; selection-background-color: #10416d; }
        QTableWidget::item { padding: 7px 3px; }
        QListWidget::item { padding: 8px; border-bottom: 1px solid #142238; }
        QListWidget::item:selected { background: #103557; color: #b5e6ff; }
        QGraphicsView { background: #eef2f6; border: none; }
        QSlider::groove:horizontal { height: 5px; background: #20334b; border-radius: 2px; }
        QSlider::handle:horizontal { width: 13px; margin: -5px 0; border-radius: 6px; background: #169ef5; }
        QSplitter::handle { background: #070e18; }
        QScrollBar:vertical { width: 8px; background: #0b1624; }
        QScrollBar::handle:vertical { background: #2b405c; border-radius: 4px; min-height: 24px; }
    )");
    auto* central=new QWidget; central->setObjectName("workspace"); setCentralWidget(central);
    auto* layout=new QVBoxLayout(central); layout->setContentsMargins(12,10,12,8); layout->setSpacing(12);
    auto* header=new QFrame; header->setObjectName("header"); auto* headerLayout=new QHBoxLayout(header);
    auto* branding=new QVBoxLayout;
    auto* brand=new QLabel("OrbitWatch"); brand->setObjectName("brand"); branding->addWidget(brand);
    auto* subtitle=new QLabel("Satellite Proximity Analysis"); subtitle->setObjectName("muted"); branding->addWidget(subtitle);
    headerLayout->addLayout(branding); headerLayout->addStretch();
    auto* modeLabel=new QLabel("SCENARIO WORKSPACE"); modeLabel->setObjectName("muted"); headerLayout->addWidget(modeLabel); headerLayout->addSpacing(28);
    auto* search=new QLineEdit; search->setObjectName("objectSearch"); search->setPlaceholderText("Search loaded objects…"); search->setClearButtonEnabled(true); search->setFixedWidth(260); headerLayout->addWidget(search);
    layout->addWidget(header);
    auto* columns=new QSplitter(Qt::Horizontal); columns->setChildrenCollapsible(false); columns->setHandleWidth(10); layout->addWidget(columns,1);
    auto card=[](const QString& title,QVBoxLayout* parent) {
        auto* frame=new QFrame; frame->setObjectName("panel"); auto* contents=new QVBoxLayout(frame);
        contents->setContentsMargins(14,14,14,14); contents->setSpacing(10);
        auto* heading=new QLabel(title); heading->setObjectName("sectionTitle"); contents->addWidget(heading);
        parent->addWidget(frame); return contents;
    };
    auto* left=new QWidget; left->setMinimumWidth(215); left->setMaximumWidth(300);
    auto* leftLayout=new QVBoxLayout(left); leftLayout->setContentsMargins(0,0,0,0); leftLayout->setSpacing(12);
    auto* settings=card("Data & Analysis",leftLayout);
    open_=new QPushButton("Load JSON"); demo100_=new QPushButton("Demo (100)"); demo_=new QPushButton("Demo (500)");
    settings->addWidget(open_); settings->addWidget(demo100_); settings->addWidget(demo_);
    settings->addWidget(new QLabel("Proximity threshold"));
    threshold_=new QDoubleSpinBox; threshold_->setRange(0.001,1e9); threshold_->setDecimals(3); threshold_->setValue(10000); threshold_->setSuffix(" m"); settings->addWidget(threshold_);
    auto* actions=new QHBoxLayout; analyze_=new QPushButton("Analyze"); analyze_->setObjectName("analyzeButton"); auto* cancel=new QPushButton("Cancel"); actions->addWidget(analyze_); actions->addWidget(cancel); settings->addLayout(actions);
    auto* projectionRow=new QHBoxLayout; projectionRow->addWidget(new QLabel("2D projection")); projection_=new QComboBox; projection_->addItems({"XY","XZ","YZ"}); projectionRow->addWidget(projection_); settings->addLayout(projectionRow);
    auto* objectsCard=card("Loaded objects",leftLayout); objectsTitle_=qobject_cast<QLabel*>(objectsCard->itemAt(0)->widget());
    objects_=new QListWidget; objects_->setObjectName("objectsList"); objectsCard->addWidget(objects_,1);
    leftLayout->setStretch(1,1);
    columns->addWidget(left);
    auto* middle=new QWidget; auto* middleLayout=new QVBoxLayout(middle); middleLayout->setContentsMargins(0,0,0,0); middleLayout->setSpacing(10);
    scene_=new QGraphicsScene(this); view_=new QGraphicsView(scene_); view_->setRenderHint(QPainter::Antialiasing);
    auto* tabs=new QTabWidget; tabs->setObjectName("viewTabs");
#ifdef ORBITWATCH_HAS_3D
    tabs->addTab(new EarthView,"3D View");
#endif
    tabs->addTab(view_,"2D Analysis"); middleLayout->addWidget(tabs,1);
    auto* playbackPanel=new QFrame; playbackPanel->setObjectName("panel"); auto* playback=new QHBoxLayout(playbackPanel);
    auto* previous=new QPushButton("‹"); auto* play=new QPushButton("Play / Pause"); auto* next=new QPushButton("›");
    previous->setToolTip("Previous sample"); next->setToolTip("Next sample");
    playback->addWidget(previous); playback->addWidget(play); playback->addWidget(next);
    time_=new QSlider(Qt::Horizontal); playback->addWidget(time_,1); clock_=new QLabel; playback->addWidget(clock_); middleLayout->addWidget(playbackPanel);
    auto* referenceNote=new QLabel("3D: Earth reference only  •  2D: loaded trajectories and sampled encounters"); referenceNote->setObjectName("muted"); referenceNote->setWordWrap(true); middleLayout->addWidget(referenceNote);
    columns->addWidget(middle);
    auto* right=new QWidget; right->setMinimumWidth(330); right->setMaximumWidth(480);
    auto* rightLayout=new QVBoxLayout(right); rightLayout->setContentsMargins(0,0,0,0); rightLayout->setSpacing(12);
    auto* eventsCard=card("Proximity events",rightLayout); eventsTitle_=qobject_cast<QLabel*>(eventsCard->itemAt(0)->widget());
    table_=new QTableWidget; table_->setColumnCount(6); table_->setHorizontalHeaderLabels({"Object A","Object B","Start (s)","End (s)","Time (s)","Min dist (m)"});
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers); table_->setSelectionBehavior(QAbstractItemView::SelectRows); table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->verticalHeader()->hide(); table_->setShowGrid(false); table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setColumnHidden(2,true); table_->setColumnHidden(3,true); table_->horizontalHeader()->moveSection(5,4);
    eventsCard->addWidget(table_,1); rightLayout->setStretch(0,1);
    auto* selectedCard=card("Selected event",rightLayout);
    details_=new QLabel("Run analysis and select an event."); details_->setObjectName("selectedEventDetails"); details_->setWordWrap(true); details_->setTextInteractionFlags(Qt::TextSelectableByMouse); selectedCard->addWidget(details_);
    auto* focus=new QPushButton("View event in 2D"); focus->setObjectName("focusEventButton"); selectedCard->addWidget(focus);
    auto* exportCard=card("Export report",rightLayout); auto* exports=new QHBoxLayout;
    export_=new QPushButton("JSON"); exportCsv_=new QPushButton("CSV"); export_->setEnabled(false); exportCsv_->setEnabled(false); exports->addWidget(export_); exports->addWidget(exportCsv_); exportCard->addLayout(exports);
    columns->addWidget(right); columns->setStretchFactor(1,1); columns->setSizes({240,820,390});
    status_=new QLabel; status_->setObjectName("workspaceStatus"); layout->addWidget(status_);
    timer_=new QTimer(this); timer_->setInterval(33);
    connect(timer_,&QTimer::timeout,this,[this]{ if(time_->value()==time_->maximum()) timer_->stop(); else time_->setValue(time_->value()+1); });
    connect(play,&QPushButton::clicked,this,[this]{ if(timer_->isActive()) timer_->stop(); else timer_->start(); });
    connect(previous,&QPushButton::clicked,this,[this]{ time_->setValue(time_->value()-1); });
    connect(next,&QPushButton::clicked,this,[this]{ time_->setValue(time_->value()+1); });
    connect(time_,&QSlider::valueChanged,this,[this]{ render(); });
    connect(projection_,&QComboBox::currentIndexChanged,this,[this]{ render(); });
    connect(search,&QLineEdit::textChanged,this,[this](const QString& text){ for(int i=0;i<objects_->count();++i) objects_->item(i)->setHidden(!objects_->item(i)->text().contains(text,Qt::CaseInsensitive)); });
    connect(open_,&QPushButton::clicked,this,[this]{ auto path=QFileDialog::getOpenFileName(this,"Open scenario",{},"JSON (*.json)"); if(path.isEmpty()) return; try { auto s=read_scenario(path); setScenario(std::move(s),path); } catch(const std::exception& e) { showError(e.what()); } });
    connect(demo_,&QPushButton::clicked,this,[this]{ setScenario(make_demo(500)); });
    connect(demo100_,&QPushButton::clicked,this,[this]{ setScenario(make_demo(100)); });
    connect(analyze_,&QPushButton::clicked,this,[this]{ run(); });
    connect(cancel,&QPushButton::clicked,this,[this]{ if(cancel_) cancel_->store(true); });
    connect(export_,&QPushButton::clicked,this,[this]{ exportReport("json"); });
    connect(exportCsv_,&QPushButton::clicked,this,[this]{ exportReport("csv"); });
    connect(threshold_,&QDoubleSpinBox::valueChanged,this,[this]{ has_result_=false; export_->setEnabled(false); exportCsv_->setEnabled(false); selected_=-1; table_->setRowCount(0); render(); });
    connect(table_,&QTableWidget::cellClicked,this,[this](int row,int){ selected_=row; timer_->stop(); time_->setValue(static_cast<int>(result_.events.at(row).minimum)); render(); });
    connect(focus,&QPushButton::clicked,this,[this,tabs]{ if(has_result_ && selected_>=0) { tabs->setCurrentWidget(view_); time_->setValue(static_cast<int>(result_.events.at(selected_).minimum)); render(); } });
    connect(&watcher_,&QFutureWatcher<Result>::finished,this,[this]{
        analyze_->setEnabled(true); open_->setEnabled(true); demo_->setEnabled(true); demo100_->setEnabled(true); threshold_->setEnabled(true);
        if(cancel_ && cancel_->load()) { status_->setText("Analysis cancelled"); return; }
        try { result_=watcher_.result(); has_result_=true; table_->setRowCount(static_cast<int>(result_.events.size()));
            int row=0; for(const auto& e:result_.events) {
                QStringList values{QString::fromStdString(e.first),QString::fromStdString(e.second),QString::number(scenario_->times_us[e.start]/1e6,'f',3),QString::number(scenario_->times_us[e.end]/1e6,'f',3),QString::number(scenario_->times_us[e.minimum]/1e6,'f',3),QString::number(e.distance_m,'f',3)};
                for(int col=0;col<6;++col) table_->setItem(row,col,new QTableWidgetItem(values[col])); ++row;
            }
            export_->setEnabled(true); exportCsv_->setEnabled(true);
            if(!result_.events.empty()) { selected_=0; table_->selectRow(0); time_->setValue(static_cast<int>(result_.events[0].minimum)); }
            render();
        } catch(const std::exception& e) { status_->setText(e.what()); } catch(...) { status_->setText("Analysis failed"); }
    });
    cancel->setObjectName("cancelButton"); demo_->setObjectName("demoButton"); demo100_->setObjectName("demo100Button");
    analyze_->setObjectName("analyzeButton"); export_->setObjectName("exportButton"); exportCsv_->setObjectName("exportCsvButton");
    threshold_->setObjectName("thresholdInput"); table_->setObjectName("eventsTable"); time_->setObjectName("timeSlider"); projection_->setObjectName("projectionSelector");
    connect(tabs,&QTabWidget::currentChanged,this,[this,tabs](int){ projection_->setEnabled(tabs->currentWidget()==view_); render(); });
    setScenario(make_demo(100)); projection_->setEnabled(tabs->currentWidget()==view_);
}

Window::~Window() {
    if(cancel_) cancel_->store(true);
    // Qt rethrows worker exceptions from waitForFinished; destructors must not throw.
    try { watcher_.waitForFinished(); } catch(...) {}
}
void Window::showError(const QString& message) { QMessageBox::warning(this,"OrbitWatch",message); }
void Window::setScenario(Scenario s,QString source) {
    validate(s); timer_->stop(); scenario_=std::make_shared<const Scenario>(std::move(s)); source_=source;
    has_result_=false; selected_=-1; result_={}; table_->setRowCount(0); export_->setEnabled(false); exportCsv_->setEnabled(false);
    objects_->clear();
    const auto filter=findChild<QLineEdit*>("objectSearch")->text();
    for(const auto& object:scenario_->objects) { auto* item=new QListWidgetItem(QString::fromStdString(object.id),objects_); item->setHidden(!item->text().contains(filter,Qt::CaseInsensitive)); }
    objectsTitle_->setText(QString("Objects (%1)").arg(scenario_->objects.size()));
    extents_={1,1,1};
    for(const auto& o:scenario_->objects) for(const auto& pos:o.positions) for(int axis=0;axis<3;++axis) extents_[axis]=std::max(extents_[axis],std::abs(pos[axis])/1000);
    time_->setRange(0,static_cast<int>(scenario_->times_us.size()-1)); time_->setValue(0); render();
}
void Window::run() {
    timer_->stop(); has_result_=false; selected_=-1; table_->setRowCount(0); export_->setEnabled(false); exportCsv_->setEnabled(false);
    analyze_->setEnabled(false); open_->setEnabled(false); demo_->setEnabled(false); demo100_->setEnabled(false); threshold_->setEnabled(false);
    updateDetails(); eventsTitle_->setText("Proximity events — analyzing…");
    status_->setText("Analyzing in background…"); cancel_=std::make_shared<std::atomic_bool>(false);
    auto s=scenario_; auto flag=cancel_; double threshold=threshold_->value();
    watcher_.setFuture(QtConcurrent::run([s,flag,threshold]{ return analyze(*s,threshold,[flag]{return flag->load();}); }));
}
void Window::render() {
    if(!scenario_) return;
    scene_->clear(); auto k=static_cast<std::size_t>(time_->value());
    const int p=projection_->currentIndex(), x=p==2?1:0, y=p==0?1:2;
    // Scene coordinates in km; scientific state remains in metres.
    double extent=std::max(extents_[x],extents_[y]);
    extent*=1.05; scene_->setSceneRect(-extent,-extent,2*extent,2*extent);
    QPen axesPen(Qt::lightGray); axesPen.setCosmetic(true);
    scene_->addLine(-extent,0,extent,0,axesPen); scene_->addLine(0,-extent,0,extent,axesPen);
    QString first,second;
    if(has_result_ && selected_>=0) { const auto& e=result_.events.at(selected_); first=QString::fromStdString(e.first); second=QString::fromStdString(e.second); }
    double radius=extent/160;
    for(const auto& o:scenario_->objects) {
        const QString id=QString::fromStdString(o.id); bool selected=id==first || id==second;
        if(selected) { QPainterPath path; bool initial=true; for(const auto& pos:o.positions) { QPointF pt(pos[x]/1000,-pos[y]/1000); if(initial){path.moveTo(pt);initial=false;}else path.lineTo(pt); } QPen pathPen(Qt::darkRed); pathPen.setCosmetic(true); pathPen.setWidthF(1.5); auto* item=scene_->addPath(path,pathPen); item->setOpacity(.5); item->setZValue(1); }
        const auto& pos=o.positions[k]; auto* item=scene_->addEllipse(pos[x]/1000-radius,-pos[y]/1000-radius,2*radius,2*radius,QPen(Qt::NoPen),QBrush(selected?Qt::red:QColor(35,110,210))); item->setToolTip(id); if(selected) item->setZValue(2);
    }
    view_->fitInView(scene_->sceneRect(),Qt::KeepAspectRatio);
    clock_->setText(QString("%1 / %2 s").arg(scenario_->times_us[k]/1e6,0,'f',0).arg(scenario_->times_us.back()/1e6,0,'f',0));
    eventsTitle_->setText(has_result_?QString("Proximity events (%1)").arg(result_.events.size()):"Proximity events — not analyzed");
    status_->setText(QString("Ready    •    Objects: %1    |    Events: %2    |    Sample: %3 / %4    |    Distances: 3D, sampled")
        .arg(scenario_->objects.size()).arg(has_result_?QString::number(result_.events.size()):QString("—")).arg(k+1).arg(scenario_->times_us.size()));
    updateDetails();
}
void Window::updateDetails() {
    auto* focus=findChild<QPushButton*>("focusEventButton");
    const bool valid=has_result_ && selected_>=0 && static_cast<std::size_t>(selected_)<result_.events.size();
    focus->setEnabled(valid);
    if(!valid) { details_->setText(has_result_?"No event selected.":"Run analysis and select an event."); return; }
    const auto& event=result_.events.at(selected_);
    auto position=[&](const std::string& id) {
        const auto it=std::find_if(scenario_->objects.begin(),scenario_->objects.end(),[&](const Object& object){return object.id==id;});
        if(it==scenario_->objects.end()) return QString("Unavailable");
        const auto& p=it->positions[event.minimum];
        return QString("(%1, %2, %3) km").arg(p[0]/1000,0,'f',2).arg(p[1]/1000,0,'f',2).arg(p[2]/1000,0,'f',2);
    };
    details_->setText(QString("<p><b>Object A</b> &nbsp; %1<br><b>Object B</b> &nbsp; %2</p>"
        "<p>Minimum distance &nbsp; <b>%3 m</b><br>Closest sample &nbsp; <b>%4 s</b><br>Sampled interval &nbsp; %5–%6 s</p>"
        "<p>Position A<br>%7<br>Position B<br>%8</p>")
        .arg(QString::fromStdString(event.first).toHtmlEscaped(),QString::fromStdString(event.second).toHtmlEscaped())
        .arg(event.distance_m,0,'f',3).arg(scenario_->times_us[event.minimum]/1e6,0,'f',3)
        .arg(scenario_->times_us[event.start]/1e6,0,'f',3).arg(scenario_->times_us[event.end]/1e6,0,'f',3)
        .arg(position(event.first),position(event.second)));
}
void Window::exportReport(const QString& format) {
    if(!has_result_) return;
    auto path=QFileDialog::getSaveFileName(this,"Export report", "report."+format,format=="csv"?"CSV (*.csv)":"JSON (*.json)");
    if(path.isEmpty()) return;
    if(!path.endsWith("."+format,Qt::CaseInsensitive)) path+="."+format;
    try { write_report(*scenario_,result_,source_,path); } catch(const std::exception& e) { showError(e.what()); }
}
}
