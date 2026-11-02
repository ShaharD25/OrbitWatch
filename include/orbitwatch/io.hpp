#pragma once
#include "core.hpp"
#include <QString>
namespace orbitwatch {
Scenario read_scenario(const QString& path);
void write_scenario(const Scenario& scenario, const QString& path);
void write_report(const Scenario& scenario, const Result& result, const QString& source, const QString& path);
}
