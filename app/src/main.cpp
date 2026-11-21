#include "../../include/MonitoringApp.h"
#include "../../include/IPercentileTracker.h"
#include "../../src/structB/include/PercentileTrackerB.h"
#include "TrackerAAdapter.h"
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>

void runEvalMode(const std::string& structType, int windowSize) {
    std::vector<std::string> lines;
    std::string line;
    std::vector<double> allValues;

    // 1. Read all input and collect potential values (required by Structure B's Fenwick Tree)
    while (std::getline(std::cin, line)) {
        if (line.empty() || line[0] == '#') continue;
        lines.push_back(line);

        std::istringstream iss(line);
        std::string op;
        iss >> op;
        if (op == "ADD" || op == "INSERT" || op == "UPDATE") {
            double first = 0.0, second = 0.0;
            if (iss >> first) {
                if (iss >> second) {
                    allValues.push_back(second); // Format: OP id value
                } else {
                    allValues.push_back(first);  // Format: ADD value (auto-ID)
                }
            }
        }
    }

    // 2. Initialize the chosen structure
    std::unique_ptr<IPercentileTracker> tracker;
    bool isStructA = (structType == "A" || structType == "a");
    if (isStructA) {
        tracker = std::make_unique<TrackerAAdapter>(windowSize);
    } else {
        tracker = std::make_unique<PercentileTrackerB>(allValues);
    }

    // 3. Process commands
    std::cout << std::fixed << std::setprecision(6);
    int64_t autoId = 1;
    std::deque<int64_t> liveIdsB; // Tracks live window order for Structure B

    for (const auto& l : lines) {
        std::istringstream iss(l);
        std::string op;
        iss >> op;

        if (op == "ADD" || op == "INSERT") {
            double first = 0.0, second = 0.0;
            int64_t id = 0;
            double val = 0.0;

            if (iss >> first) {
                if (iss >> second) {
                    id = static_cast<int64_t>(first);
                    val = second;
                } else {
                    id = autoId++;
                    val = first;
                }
            }

            if (!isStructA) {
                auto it = std::find(liveIdsB.begin(), liveIdsB.end(), id);
                if (it != liveIdsB.end()) {
                    liveIdsB.erase(it);
                    tracker->erase(id);
                }
                liveIdsB.push_back(id);
                if (windowSize > 0 && static_cast<int>(liveIdsB.size()) > windowSize) {
                    int64_t expiredId = liveIdsB.front();
                    liveIdsB.pop_front();
                    tracker->erase(expiredId);
                }
            }

            tracker->insert(id, val);
        } else if (op == "RETRACT" || op == "ERASE") {
            int64_t id;
            if (iss >> id) {
                if (!isStructA) {
                    auto it = std::find(liveIdsB.begin(), liveIdsB.end(), id);
                    if (it != liveIdsB.end()) {
                        liveIdsB.erase(it);
                    }
                }
                tracker->erase(id);
            }
        } else if (op == "UPDATE") {
            int64_t id;
            double val;
            if (iss >> id >> val) {
                tracker->update(id, val);
            }
        } else if (op == "QUERY") {
            double p = 50.0;
            if (!(iss >> p)) p = 50.0;
            std::cout << tracker->getPercentile(p) << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    std::string mode = "dashboard";
    std::string structType = "B";
    std::string inputFile = "";
    int windowSize = 100000; // Default window size

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--eval") mode = "eval";
        else if (arg == "--structure" && i + 1 < argc) structType = argv[++i];
        else if (arg == "--input" && i + 1 < argc) inputFile = argv[++i];
        else if (arg == "--window" && i + 1 < argc) windowSize = std::stoi(argv[++i]); // Parse --window flag
    }

    if (mode == "eval") {
        runEvalMode(structType, windowSize);
        return 0;
    }

    std::cout << "Starting Dashboard Mode...\n";
    std::vector<TelemetryEvent> eventStream = {
        {TelemetryEvent::Type::INSERT,  1001, 45.2,  "auth-service"},
        {TelemetryEvent::Type::INSERT,  1002, 52.8,  "auth-service"},
        {TelemetryEvent::Type::INSERT,  1004, 110.5, "search-indexer"},
        {TelemetryEvent::Type::QUERY,   0,    0.0,   "system"}
    };

    std::vector<double> initVals;
    for (const auto& ev : eventStream) {
        if (ev.type == TelemetryEvent::Type::INSERT || ev.type == TelemetryEvent::Type::UPDATE)
            initVals.push_back(ev.value);
    }

    std::unique_ptr<IPercentileTracker> engine;
    if (structType == "A" || structType == "a") {
        engine = std::make_unique<TrackerAAdapter>(windowSize); // Use dynamic window
    } else {
        engine = std::make_unique<PercentileTrackerB>(initVals);
    }

    MonitoringApp app(std::move(engine), 200.0);
    for (const auto& ev : eventStream) {
        app.processEvent(ev);
    }

    return 0;
}
