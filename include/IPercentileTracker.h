#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Abstract interface for the Running Median / Percentile Engine with Retractions.
 * Decouples the Application layer from underlying data structure implementations (Struct A / Struct B).
 *
 * Architectural Contract (see INTERFACE.md):
 * - Empty Window (size() == 0): getMedian() and getPercentile(p) MUST return -1.0.
 * - Even Element Count (size() % 2 == 0): getMedian() MUST return (v_{N/2 - 1} + v_{N/2}) / 2.0.
 */
class IPercentileTracker {
public:
    virtual ~IPercentileTracker() = default;

    // Core dynamic stream operations
    virtual void insert(int64_t id, double value) = 0;
    virtual bool erase(int64_t id) = 0;
    virtual bool update(int64_t id, double newValue) = 0;

    // Statistical queries (must return -1.0 sentinel when empty())
    virtual double getMedian() const = 0;
    virtual double getPercentile(double p) const = 0; // p in [0.0, 100.0]

    // Metadata & diagnostics
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;
    virtual void clear() = 0;
    virtual std::string getEngineName() const = 0;
};
