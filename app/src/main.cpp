#include "../../include/MonitoringApp.h"
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <stdexcept>

// ============================================================================
// ADAPTER / REFERENCE ENGINE
// Bridges Struct_A / Struct_B core implementations to IPercentileTracker.
// Strictly enforces all edge-case rules defined in INTERFACE.md.
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

    double getMedian() const override {
        if (activeEvents_.empty()) return -1.0;

        std::vector<double> vals;
        vals.reserve(activeEvents_.size());
        for (const auto& pair : activeEvents_) {
            vals.push_back(pair.second);
        }
        std::sort(vals.begin(), vals.end());

        size_t n = vals.size();
        if (n % 2 == 1) {
            return vals[n / 2];
        }
        // Strict INTERFACE.md contract: even count returns arithmetic mean of two middle elements
        return (vals[n / 2 - 1] + vals[n / 2]) / 2.0;
    }

    double getPercentile(double p) const override {
        if (p < 0.0 || p > 100.0) {
            throw std::invalid_argument("Percentile p must be within [0.0, 100.0]");
        }
        // Strict INTERFACE.md contract: empty window returns -1.0 sentinel
        if (activeEvents_.empty()) return -1.0;
        if (p == 50.0) return getMedian();

        std::vector<double> vals;
        vals.reserve(activeEvents_.size());
        for (const auto& pair : activeEvents_) {
            vals.push_back(pair.second);
        }
        std::sort(vals.begin(), vals.end());

        if (p == 0.0) return vals.front();
        if (p == 100.0) return vals.back();

        size_t n = vals.size();
        size_t idx = static_cast<size_t>(std::ceil((p / 100.0) * n));
        if (idx > 0) idx -= 1;
        if (idx >= n) idx = n - 1;
        return vals[idx];
    }

    size_t size() const override { return activeEvents_.size(); }
    bool empty() const override { return activeEvents_.empty(); }
    void clear() override { activeEvents_.clear(); }

    std::string getEngineName() const override {
        return "Dynamic Percentile Engine (Retraction-Enabled)";
    }
};

// ============================================================================
// MAIN SIMULATION PIPELINE
// ============================================================================
int main() {
    std::cout << "Starting Real-Time SLA & Latency Monitoring Pipeline...\n\n";

    auto engine = std::make_unique<ReferenceRetractionEngine>();
    MonitoringApp app(std::move(engine), 200.0);

    // Verify empty-window sentinel behavior before any events arrive
    app.processEvent({TelemetryEvent::Type::QUERY, 0, 0.0, "system"});

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
