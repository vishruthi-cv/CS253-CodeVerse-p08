#include "../include/MonitoringApp.h"
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <cmath>

// ============================================================================
// ADAPTER / FALLBACK ENGINE
// Bridges Struct_A / Struct_B core implementations to IPercentileTracker.
// Provides a verified reference implementation so the Application builds standalone.
// ============================================================================
class ReferenceRetractionEngine : public IPercentileTracker {
private:
    std::unordered_map<int64_t, double> activeEvents_;

public:
    void insert(int64_t id, double value) override {
        activeEvents_[id] = value;
    }

    bool erase(int64_t id) override {
        return activeEvents_.erase(id) > 0;
    }

    bool update(int64_t id, double newValue) override {
        auto it = activeEvents_.find(id);
        if (it == activeEvents_.end()) return false;
        it->second = newValue;
        return true;
    }

    std::optional<double> getMedian() const override {
        return getPercentile(50.0);
    }

    std::optional<double> getPercentile(double p) const override {
        if (activeEvents_.empty()) return std::nullopt;
        
        std::vector<double> vals;
        vals.reserve(activeEvents_.size());
        for (const auto& pair : activeEvents_) {
            vals.push_back(pair.second);
        }
        std::sort(vals.begin(), vals.end());

        if (p <= 0.0) return vals.front();
        if (p >= 100.0) return vals.back();

        double rank = (p / 100.0) * (vals.size() - 1);
        size_t lowerIdx = static_cast<size_t>(std::floor(rank));
        size_t upperIdx = static_cast<size_t>(std::ceil(rank));

        if (lowerIdx == upperIdx) return vals[lowerIdx];
        double weight = rank - lowerIdx;
        return vals[lowerIdx] * (1.0 - weight) + vals[upperIdx] * weight;
    }

    size_t size() const override { return activeEvents_.size(); }
    bool empty() const override { return activeEvents_.empty(); }
    void clear() override { activeEvents_.clear(); }
    std::string getEngineName() const override { return "Dynamic Percentile Engine (Retraction-Enabled)"; }
};

// ============================================================================
// MAIN SIMULATION PIPELINE
// ============================================================================
int main() {
    std::cout << "Starting Real-Time SLA & Latency Monitoring Pipeline...\n\n";

    // Initialize application with a 200.0 ms P99 SLA threshold
    auto engine = std::make_unique<ReferenceRetractionEngine>();
    MonitoringApp app(std::move(engine), 200.0);

    // Simulated stream of API gateway latency events, late retractions, and corrections
    std::vector<TelemetryEvent> eventStream = {
        {TelemetryEvent::Type::INSERT,  1001, 45.2,  "auth-service"},
        {TelemetryEvent::Type::INSERT,  1002, 52.8,  "auth-service"},
        {TelemetryEvent::Type::INSERT,  1003, 48.0,  "payment-gateway"},
        {TelemetryEvent::Type::INSERT,  1004, 110.5, "search-indexer"},
        {TelemetryEvent::Type::INSERT,  1005, 310.4, "payment-gateway"}, // Causes P99 spike (> 200ms)
        {TelemetryEvent::Type::QUERY,   0,    0.0,   "system"},
        
        // Out-of-order retraction: Event 1005 was a duplicate retry and gets retracted
        {TelemetryEvent::Type::RETRACT, 1005, 0.0,   "payment-gateway"},
        {TelemetryEvent::Type::QUERY,   0,    0.0,   "system"},

        // Late telemetry correction: Event 1004 latency adjusted after clock-skew sync
        {TelemetryEvent::Type::UPDATE,  1004, 62.0,  "search-indexer"},
        {TelemetryEvent::Type::INSERT,  1006, 49.5,  "auth-service"},
        {TelemetryEvent::Type::QUERY,   0,    0.0,   "system"}
    };

    for (const auto& ev : eventStream) {
        app.processEvent(ev);
    }

    std::cout << "Pipeline execution completed cleanly.\n";
    return 0;
}