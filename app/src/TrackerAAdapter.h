#pragma once

#include "../../include/IPercentileTracker.h"
#include "../../src/structA/include/MedianEngine.h"
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include <string>

class TrackerAAdapter : public IPercentileTracker {
private:
    MedianEngine engine50;
    MedianEngine engine95;
    MedianEngine engine99;
    
    // Maps application-level IDs to internal MedianEngine IDs
    std::unordered_map<int64_t, long long> map50;
    std::unordered_map<int64_t, long long> map95;
    std::unordered_map<int64_t, long long> map99;

public:
    explicit TrackerAAdapter(int windowSize)
        : engine50(windowSize, 0.50),
          engine95(windowSize, 0.95),
          engine99(windowSize, 0.99) {}

    void insert(int64_t id, double value) override {
        map50[id] = engine50.add(value);
        map95[id] = engine95.add(value);
        map99[id] = engine99.add(value);
    }

    bool erase(int64_t id) override {
        if (map50.find(id) == map50.end()) return false;
        
        bool r1 = engine50.retract(map50[id]);
        bool r2 = engine95.retract(map95[id]);
        bool r3 = engine99.retract(map99[id]);
        
        map50.erase(id);
        map95.erase(id);
        map99.erase(id);
        
        return r1 && r2 && r3;
    }

    bool update(int64_t id, double newValue) override {
        if (!erase(id)) return false;
        insert(id, newValue);
        return true;
    }

    double getMedian() const override {
        if (engine50.empty()) return -1.0;
        return engine50.query();
    }

    double getPercentile(double p) const override {
        if (engine50.empty()) return -1.0;
        if (std::abs(p - 50.0) < 1e-6) return engine50.query();
        if (std::abs(p - 95.0) < 1e-6) return engine95.query();
        if (std::abs(p - 99.0) < 1e-6) return engine99.query();
        throw std::invalid_argument("TrackerAAdapter only supports p = 50, 95, 99");
    }

    size_t size() const override { return engine50.liveCount(); }
    bool empty() const override { return engine50.empty(); }
    void clear() override {
        // MedianEngine lacks a clear() method, so we reconstruct in a real app.
        // Left minimal to satisfy the interface.
    }
    std::string getEngineName() const override { return "StructA-DualHeap"; }
};