#pragma once
#include "IPercentileTracker.h"
#include <memory>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

struct TelemetryEvent {
    enum class Type { INSERT, RETRACT, UPDATE, QUERY };
    Type type;
    int64_t eventId;
    double value;
    std::string sourceNode;
};

/**
 * @brief Real-Time Telemetry & SLA Monitoring Pipeline.
 * Consumes streaming metrics, handles out-of-order retractions/corrections,
 * and evaluates P50 (Median), P95, and P99 latency SLAs in real time.
 */
class MonitoringApp {
private:
    std::unique_ptr<IPercentileTracker> tracker_;
    double slaThresholdP99_;
    size_t totalProcessed_ = 0;
    size_t totalRetracted_ = 0;
    size_t slaViolations_ = 0;

public:
    MonitoringApp(std::unique_ptr<IPercentileTracker> tracker, double slaThresholdP99 = 250.0)
        : tracker_(std::move(tracker)), slaThresholdP99_(slaThresholdP99) {}

    void processEvent(const TelemetryEvent& event) {
        switch (event.type) {
            case TelemetryEvent::Type::INSERT:
                tracker_->insert(event.eventId, event.value);
                totalProcessed_++;
                checkSLA();
                break;

            case TelemetryEvent::Type::RETRACT:
                if (tracker_->erase(event.eventId)) {
                    totalRetracted_++;
                    std::cout << "[RETRACT] Event ID " << event.eventId 
                              << " removed (" << event.sourceNode << "). Recalculating metrics...\n";
                    checkSLA();
                } else {
                    std::cout << "[WARN] Retraction failed: Event ID " << event.eventId << " not found.\n";
                }
                break;

            case TelemetryEvent::Type::UPDATE:
                if (tracker_->update(event.eventId, event.value)) {
                    std::cout << "[UPDATE] Event ID " << event.eventId 
                              << " corrected to " << event.value << "ms.\n";
                    checkSLA();
                } else {
                    std::cout << "[WARN] Update failed: Event ID " << event.eventId << " not found.\n";
                }
                break;

            case TelemetryEvent::Type::QUERY:
                printDashboard();
                break;
        }
    }

    void checkSLA() {
        auto p99 = tracker_->getPercentile(99.0);
        if (p99.has_value() && p99.value() > slaThresholdP99_) {
            slaViolations_++;
            std::cout << "  [ALERT] SLA Breach! Current P99 (" << std::fixed << std::setprecision(2) 
                      << p99.value() << "ms) exceeds threshold (" << slaThresholdP99_ << "ms)\n";
        }
    }

    void printDashboard() const {
        std::cout << "\n======================================================\n";
        std::cout << " TELEMETRY ENGINE: " << tracker_->getEngineName() << "\n";
        std::cout << " Active Window Size : " << tracker_->size() << " events\n";
        std::cout << " Processed / Retract: " << totalProcessed_ << " / " << totalRetracted_ << "\n";
        std::cout << " SLA Violations     : " << slaViolations_ << "\n";
        
        if (tracker_->empty()) {
            std::cout << " Metrics            : [No active data]\n";
        } else {
            auto p50 = tracker_->getMedian();
            auto p95 = tracker_->getPercentile(95.0);
            auto p99 = tracker_->getPercentile(99.0);

            std::cout << std::fixed << std::setprecision(2);
            std::cout << " P50 (Median)       : " << (p50 ? std::to_string(*p50) + " ms" : "N/A") << "\n";
            std::cout << " P95 Latency        : " << (p95 ? std::to_string(*p95) + " ms" : "N/A") << "\n";
            std::cout << " P99 Latency        : " << (p99 ? std::to_string(*p99) + " ms" : "N/A") << "\n";
        }
        std::cout << "======================================================\n\n";
    }
};