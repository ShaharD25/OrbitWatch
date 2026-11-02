#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
namespace orbitwatch {
using Position = std::array<double, 3>;
struct Object { std::string id; std::vector<Position> positions; };
struct Scenario {
    std::string name;
    std::vector<std::int64_t> times_us;
    std::vector<Object> objects;
    std::string source_sha256;
};
struct Event {
    std::string first, second;
    std::size_t start{}, end{}, minimum{};
    double distance_m{};
};
struct Result { double threshold_m{}; std::vector<Event> events; };
void validate(const Scenario& scenario);
double distance(const Position& a, const Position& b);
Result analyze(const Scenario& scenario, double threshold_m,
               const std::function<bool()>& cancelled = {}, std::size_t event_limit = 100000);
Scenario make_demo(std::size_t count = 100, std::size_t samples = 1000);
}
