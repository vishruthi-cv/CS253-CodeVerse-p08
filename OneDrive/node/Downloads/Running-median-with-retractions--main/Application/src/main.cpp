#include "../include/MonitoringApp.h"
#include "../../Struct_A/include/MedianEngine.h"       // Adjust path if needed
#include "../../Struct_B/include/PercentileEngine.h"   // Adjust path if needed
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

// ==========================================
// ADAPTER FOR STRUCT A (MedianEngine)
// ==========================================
class StructAAdapter : public IPercentileTracker {
private:
    MedianEngine engine;
public:
    StructAAdapter(int window_size, double percentile = 0.5) 
        : engine(window_size, percentile) {}

    void insert(long long id, double value) override {
        engine.add(value); // Assuming Struct A uses add()
    }
    
    void retract(long long id) override {
        engine.retract(id); 
    }
    
    double query(double p) override {
        return engine.query();
    }
};

// ==========================================
// ADAPTER FOR STRUCT B (PercentileEngine)
// ==========================================
class StructBAdapter : public IPercentileTracker {
private:
    PercentileEngine engine;
public:
    StructBAdapter(const std::vector<double>& all_possible_values) 
        : engine(all_possible_values) {}

    void insert(long long id, double value) override {
        engine.insert(id, value);
    }
    
    void retract(long long id) override {
        engine.retract(id);
    }
    
    double query(double p) override {
        return engine.query(p);
    }
};

// ==========================================
// MOCK TRACKER FOR ORACLE
// ==========================================
class MockTracker : public IPercentileTracker {
private:
    double last_value = 0;
public:
    void insert(long long id, double value) override { last_value = value; }
    void retract(long long id) override {}
    double query(double p) override {
        if (p == 0.90) return last_value + 200; // Simulated alert trigger
        return last_value;
    }
};

// ==========================================
// MAIN FUNCTION
// ==========================================
int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // 1. Prepare Struct B's universe of values
    std::vector<double> universe_of_values;
    for(int i = 0; i <= 1000; i++) {
        universe_of_values.push_back((double)i);
    }
    std::sort(universe_of_values.begin(), universe_of_values.end());

    // 2. Instantiate real adapters and mock oracle
    auto struct_a = std::make_unique<StructAAdapter>(10, 0.5); 
    auto struct_b = std::make_unique<StructBAdapter>(universe_of_values);
    auto oracle_ptr = std::make_unique<MockTracker>();

    // 3. Initialize Monitoring App (Window Size: 10, Alert Threshold: 800.0)
    MonitoringApp app(10, 800.0, std::move(struct_a), std::move(struct_b), std::move(oracle_ptr));

    std::cout << "Starting stream processing test...\n";
    
    // Simulate incoming data stream
    for (long long i = 1; i <= 20; i++) {
        double simulated_value = (i * 100) % 1000;
        std::cout << "Reading ID " << i << " | Value: " << simulated_value << "\n";
        app.process_reading(i, simulated_value);
    }
    
    std::cout << "Stream processing complete.\n";
    return 0;
}