#pragma once
#include <vector>
#include <unordered_map>
#include "IPercentileTracker.h"
#include "PercentileEngine.h"

class PercentileTrackerB : public IPercentileTracker
{
public:
    
    PercentileTrackerB(std::vector<double> allValues);

    void insert(long long id, double value) override;
    void retract(long long id) override;
    double query(double p) override;

private:
    PercentileEngine engine;
    std::unordered_map<long long, double> valueOf;   
};