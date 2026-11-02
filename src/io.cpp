#include "orbitwatch/io.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <cmath>
#include <stdexcept>
namespace orbitwatch {
namespace {
void fail(const QString& message) { throw std::runtime_error(message.toStdString()); }
QByteArray read(const QString& path) {
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) fail(f.errorString());
    if (f.size()>256*1024*1024) fail("Input exceeds 256 MiB limit");
    return f.readAll();
}
void save(const QString& path,const QByteArray& bytes) {
    QSaveFile f(path); if (!f.open(QIODevice::WriteOnly)) fail(f.errorString());
    if (f.write(bytes)!=bytes.size() || !f.commit()) fail(f.errorString());
}
QJsonObject metadata() { return {{"schema_version",1},{"units","m"},{"time_unit","us"},{"frame","SCENARIO_CARTESIAN"}}; }
}
Scenario read_scenario(const QString& path) {
    const auto bytes=read(path);
    QJsonParseError error; auto doc=QJsonDocument::fromJson(bytes,&error);
    if (error.error!=QJsonParseError::NoError) fail(QString("JSON byte %1: %2").arg(error.offset).arg(error.errorString()));
    if (!doc.isObject()) fail("Root must be an object");
    const auto root=doc.object();
    if (root["schema_version"].toDouble(-1)!=1 || root["units"]!="m" || root["time_unit"]!="us" || root["frame"]!="SCENARIO_CARTESIAN") fail("Unsupported schema, units, time unit or frame");
    if (!root["times_us"].isArray() || !root["objects"].isArray()) fail("times_us and objects must be arrays");
    Scenario s; s.name=root["name"].toString().toStdString();
    for (const auto& v : root["times_us"].toArray()) {
        double n=v.toDouble(-1);
        if (!v.isDouble() || !std::isfinite(n) || n<0 || n>9007199254740991.0 || std::floor(n)!=n) fail("times_us must contain nonnegative exact JSON integers <= 2^53-1");
        s.times_us.push_back(static_cast<std::int64_t>(n));
    }
    for (const auto& v : root["objects"].toArray()) {
        if (!v.isObject()) fail("Each object must be a JSON object");
        auto obj=v.toObject();
        if (!obj["id"].isString() || !obj["positions"].isArray()) fail("Object requires string id and positions array");
        Object o{obj["id"].toString().toStdString(),{}};
        for (const auto& row : obj["positions"].toArray()) {
            if (!row.isArray() || row.toArray().size()!=3) fail("Each position must contain x,y,z");
            auto a=row.toArray(); Position p;
            for (int axis=0;axis<3;++axis) { if (!a[axis].isDouble()) fail("Position components must be numbers"); p[axis]=a[axis].toDouble(); }
            o.positions.push_back(p);
        }
        s.objects.push_back(std::move(o));
    }
    validate(s); s.source_sha256=QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex().toStdString(); return s;
}
void write_scenario(const Scenario& s,const QString& path) {
    validate(s); auto root=metadata(); root["name"]=QString::fromStdString(s.name);
    QJsonArray times,objects;
    for (auto t:s.times_us) { if(t>9007199254740991LL) fail("Time cannot be represented exactly in JSON"); times.append(static_cast<double>(t)); }
    for (const auto& o:s.objects) { QJsonArray positions; for(const auto& p:o.positions) positions.append(QJsonArray{p[0],p[1],p[2]}); objects.append(QJsonObject{{"id",QString::fromStdString(o.id)},{"positions",positions}}); }
    root["times_us"]=times; root["objects"]=objects; save(path,QJsonDocument(root).toJson(QJsonDocument::Compact));
}
void write_report(const Scenario& s,const Result& r,const QString& source,const QString& path) {
    if (!source.isEmpty() && (QFileInfo(source).absoluteFilePath()==QFileInfo(path).absoluteFilePath() || (!QFileInfo(path).canonicalFilePath().isEmpty() && QFileInfo(source).canonicalFilePath()==QFileInfo(path).canonicalFilePath()))) fail("Report cannot overwrite scenario source");
    QJsonArray events;
    for(const auto& e:r.events) events.append(QJsonObject{
        {"first",QString::fromStdString(e.first)},{"second",QString::fromStdString(e.second)},
        {"start_us",static_cast<double>(s.times_us.at(e.start))},{"end_us",static_cast<double>(s.times_us.at(e.end))},
        {"minimum_us",static_cast<double>(s.times_us.at(e.minimum))},{"minimum_distance_m",e.distance_m},
        {"touches_start",e.start==0},{"touches_end",e.end+1==s.times_us.size()}});
    if(path.endsWith(".csv",Qt::CaseInsensitive)) {
        auto quote=[](QString v) { v.replace(QChar('"'),QStringLiteral("\"\"")); return QChar('"')+v+QChar('"'); };
        QString csv="first,second,start_us,end_us,minimum_us,minimum_distance_m,touches_start,touches_end\n";
        for(const auto& e:r.events) csv+=quote(QString::fromStdString(e.first))+","+quote(QString::fromStdString(e.second))+","+QString::number(s.times_us.at(e.start))+","+QString::number(s.times_us.at(e.end))+","+QString::number(s.times_us.at(e.minimum))+","+QString::number(e.distance_m,'g',17)+","+(e.start==0?"true":"false")+","+(e.end+1==s.times_us.size()?"true":"false")+"\n";
        save(path,csv.toUtf8()); return;
    }
    QString hash=QString::fromStdString(s.source_sha256);
    auto root=metadata(); root["engine_version"]="0.1.0"; root["scenario"]=QString::fromStdString(s.name);
    root["source_sha256"]=hash; root["threshold_m"]=r.threshold_m; root["method"]="sampled_distance_lte";
    root["limitation"]="Sampled minimum only; encounters between samples may be missed.";
    root["object_count"]=static_cast<int>(s.objects.size()); root["sample_count"]=static_cast<int>(s.times_us.size());
    root["events"]=events; save(path,QJsonDocument(root).toJson());
}
}
