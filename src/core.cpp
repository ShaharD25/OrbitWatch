#include "orbitwatch/core.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <unordered_set>
namespace orbitwatch {
void validate(const Scenario& s) {
    if (s.objects.size() < 2 || s.objects.size() > 500 || s.times_us.size() < 2 || s.times_us.size() > 10000)
        throw std::runtime_error("Expected 2..500 objects and 2..10000 samples");
    if (s.times_us.front() != 0) throw std::runtime_error("First sample time must be zero");
    for (std::size_t k = 1; k < s.times_us.size(); ++k)
        if (s.times_us[k] <= s.times_us[k-1]) throw std::runtime_error("Sample times must strictly increase");
    std::unordered_set<std::string> ids;
    for (const auto& o : s.objects) {
        if (o.id.empty() || !ids.insert(o.id).second) throw std::runtime_error("Empty or duplicate object ID: " + o.id);
        if (o.positions.size() != s.times_us.size()) throw std::runtime_error("Sample count mismatch: " + o.id);
        for (const auto& p : o.positions) for (double v : p)
            if (!std::isfinite(v) || std::abs(v) > 1e12) throw std::runtime_error("Coordinate must be finite and within +/-1e12 m: " + o.id);
    }
}
double distance(const Position& a, const Position& b) {
    return std::hypot(a[0]-b[0], a[1]-b[1], a[2]-b[2]);
}
Result analyze(const Scenario& s, double threshold, const std::function<bool()>& cancelled, std::size_t limit) {
    validate(s);
    if (!std::isfinite(threshold) || threshold <= 0) throw std::runtime_error("Threshold must be finite and positive");
    Result result{threshold, {}};
    const double threshold2 = threshold * threshold;
    auto emit = [&](const Event& event) {
        if (result.events.size() >= limit) throw std::runtime_error("Event limit exceeded; no complete result available");
        result.events.push_back(event);
    };
    for (std::size_t i=0; i<s.objects.size(); ++i) for (std::size_t j=i+1; j<s.objects.size(); ++j) {
        std::optional<Event> active;
        for (std::size_t k=0; k<s.times_us.size(); ++k) {
            if (k % 128 == 0 && cancelled && cancelled()) throw std::runtime_error("Analysis cancelled");
            const auto& a=s.objects[i].positions[k]; const auto& b=s.objects[j].positions[k];
            double d2=0; for (int axis=0; axis<3; ++axis) { double d=a[axis]-b[axis]; d2+=d*d; }
            if (d2 <= threshold2) {
                if (!active) active=Event{s.objects[i].id,s.objects[j].id,k,k,k,d2};
                active->end=k;
                if (d2 < active->distance_m) { active->distance_m=d2; active->minimum=k; }
            } else if (active) { active->distance_m=std::sqrt(active->distance_m); emit(*active); active.reset(); }
        }
        if (active) { active->distance_m=std::sqrt(active->distance_m); emit(*active); }
    }
    for (auto& e : result.events) if (e.second < e.first) std::swap(e.first,e.second);
    std::sort(result.events.begin(),result.events.end(),[](const Event& a,const Event& b) {
        if (a.start!=b.start) return a.start<b.start;
        if (a.first!=b.first) return a.first<b.first;
        return a.second<b.second;
    });
    return result;
}
Scenario make_demo(std::size_t count, std::size_t samples) {
    if (count < 2 || count > 500 || samples < 2 || samples > 10000) throw std::runtime_error("Invalid demo size");
    Scenario s{"Synthetic circular trajectories (not propagated orbits)",{}, {}, {}};
    for (std::size_t k=0;k<samples;++k) s.times_us.push_back(static_cast<std::int64_t>(k)*1000000);
    constexpr double pi=3.14159265358979323846;
    for (std::size_t i=0;i<count;++i) {
        Object o{"OBJ-"+std::to_string(i+1),{}};
        for (std::size_t k=0;k<samples;++k) {
            double angle=2*pi*k/(samples-1)+2*pi*i/count;
            double radius=7000000.0+20000.0*(i%7);
            o.positions.push_back({radius*std::cos(angle),radius*std::sin(angle),100000.0*std::sin(angle+i)});
        }
        s.objects.push_back(std::move(o));
    }
    // Controlled encounter: object 2 passes object 1 with a 100 m minimum.
    for (std::size_t k=0;k<samples;++k) {
        auto p=s.objects[0].positions[k];
        p[0]+=2000.0*(static_cast<double>(k)-static_cast<double>(samples/2)); p[2]+=100;
        s.objects[1].positions[k]=p;
    }
    return s;
}
}
