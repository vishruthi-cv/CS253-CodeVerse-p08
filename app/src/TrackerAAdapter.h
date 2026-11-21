#pragma once

#include "../../include/IPercentileTracker.h"
#include "../../src/structA/include/MedianEngine.h"
#include <unordered_map>
#include <deque>
#include <memory>
#include <cmath>
#include <stdexcept>
#include <string>

class TrackerAAdapter : public IPercentileTracker {
private:
    struct EngineEntry {
        double p;
        MedianEngine engine;
        std::unordered_map<int64_t, long long> idMap;

        // The critical fix: pass 0 to MedianEngine to disable its internal arrival-based expiry
        // so that the adapter's liveOrder_ logic entirely controls window eviction.
        EngineEntry(int windowSize, double percentile)
            : p(percentile), engine(0, percentile / 100.0) {} 
    };

    int windowSize_;
    mutable std::deque<EngineEntry> engines_;

    // Tracks live elements in insertion order so lazily created engines can replay them
    std::deque<int64_t> liveOrder_;
    std::unordered_map<int64_t, double> liveValues_;

    EngineEntry& getOrCreateEngine(double p) const {
        for (auto& entry : engines_) {
            if (std::abs(entry.p - p) < 1e-6) {
                return entry;
            }
        }

        engines_.emplace_back(windowSize_, p);
        EngineEntry& created = engines_.back();

        for (int64_t id : liveOrder_) {
            auto it = liveValues_.find(id);
            if (it != liveValues_.end()) {
                created.idMap[id] = created.engine.add(it->second);
            }
        }
        return created;
    }

public:
    explicit TrackerAAdapter(int windowSize) : windowSize_(windowSize) {
        // Pre-create common quantiles to avoid replay overhead on standard workloads
        engines_.emplace_back(windowSize_, 50.0);
        engines_.emplace_back(windowSize_, 95.0);
        engines_.emplace_back(windowSize_, 99.0);
    }

    void insert(int64_t id, double value) override {
        if (liveValues_.find(id) != liveValues_.end()) {
            erase(id);
        }

        liveOrder_.push_back(id);
        liveValues_[id] = value;

        if (windowSize_ > 0 && static_cast<int>(liveOrder_.size()) > windowSize_) {
            int64_t expiredId = liveOrder_.front();
            liveOrder_.pop_front();
            liveValues_.erase(expiredId);

            for (auto& entry : engines_) {
                auto it = entry.idMap.find(expiredId);
                if (it != entry.idMap.end()) {
                    entry.engine.retract(it->second);
                    entry.idMap.erase(it);
                }
            }
        }

        for (auto& entry : engines_) {
            entry.idMap[id] = entry.engine.add(value);
        }
    }

    bool erase(int64_t id) override {
        auto valIt = liveValues_.find(id);
        if (valIt == liveValues_.end()) return false;

        liveValues_.erase(valIt);
        for (auto it = liveOrder_.begin(); it != liveOrder_.end(); ++it) {
            if (*it == id) {
                liveOrder_.erase(it);
                break;
            }
        }

        bool allRetracted = true;
        for (auto& entry : engines_) {
            auto mapIt = entry.idMap.find(id);
            if (mapIt != entry.idMap.end()) {
                if (!entry.engine.retract(mapIt->second)) {
                    allRetracted = false;
                }
                entry.idMap.erase(mapIt);
            }
        }
        return allRetracted;
    }

    bool update(int64_t id, double newValue) override {
        if (liveValues_.find(id) == liveValues_.end()) return false;
        liveValues_[id] = newValue;

        bool allUpdated = true;
        for (auto& entry : engines_) {
            auto mapIt = entry.idMap.find(id);
            if (mapIt != entry.idMap.end()) {
                if (!entry.engine.retract(mapIt->second)) {
                    allUpdated = false;
                }
                entry.idMap[id] = entry.engine.add(newValue);
            }
        }
        return allUpdated;
    }

    double getMedian() const override {
        return getPercentile(50.0);
    }

    double getPercentile(double p) const override {
        if (p <= 0.0 || p > 100.0) {
            throw std::invalid_argument("Percentile p must be in (0, 100]");
        }
        if (liveValues_.empty()) return -1.0;

        EngineEntry& entry = getOrCreateEngine(p);
        if (entry.engine.empty()) return -1.0;
        return entry.engine.query();
    }

    size_t size() const override {
        return liveValues_.size();
    }

    bool empty() const override {
        return liveValues_.empty();
    }

    void clear() override {
        liveOrder_.clear();
        liveValues_.clear();
        engines_.clear();
        engines_.emplace_back(windowSize_, 50.0);
        engines_.emplace_back(windowSize_, 95.0);
        engines_.emplace_back(windowSize_, 99.0);
    }

    std::string getEngineName() const override {
        return "StructA-DualHeap";
    }
};
