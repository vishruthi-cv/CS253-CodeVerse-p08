#include "../../include/MonitoringApp.h"
#include "../../include/IPercentileTracker.h"
#include "../../src/structB/include/PercentileTrackerB.h"
#include "TrackerAAdapter.h"
#include <iostream>
#include <vector>
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
            int64_t id;
            double val;
            if (op == "UPDATE") { iss >> id >> val; } 
            else { iss >> val; } // Handle ADD format based on final instructor specs
            allValues.push_back(val);
        }
    }

    // 2. Initialize the chosen structure
    std::unique_ptr<IPercentileTracker> tracker;
    if (structType == "A" || structType == "a") {
        tracker = std::make_unique<TrackerAAdapter>(windowSize); // Use dynamic window
    } else {
        tracker = std::make_unique<PercentileTrackerB>(allValues);
    }

    // 3. Process commands
    std::cout << std::fixed << std::setprecision(6);
    int64_t autoId = 1; // Fallback ID if input lacks it

    for (const auto& l : lines) {
        std::istringstream iss(l);
        std::string op;
        iss >> op;

        if (op == "ADD" || op == "INSERT") {
            double val;
            iss >> val;
            uint64_t currentId = autoId++;
            tracker->insert(currentId, val);
            
            // Fix: Expire id-window for Structure B so it matches Structure A's window
            if ((structType == "B" || structType == "b") && currentId > (uint64_t)windowSize) {
                tracker->erase(currentId - windowSize);
            }
        } else if (op == "RETRACT" || op == "ERASE") {
            int64_t id;
            iss >> id;
            tracker->erase(id);
        } else if (op == "UPDATE") {
            int64_t id;
            double val;
            iss >> id >> val;
            tracker->update(id, val);
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
