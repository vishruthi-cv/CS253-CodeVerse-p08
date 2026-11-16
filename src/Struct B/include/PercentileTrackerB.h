#pragma once
#include <vector>
#include "IPercentileTracker.h"
#include "PercentileEngine.h"
#include "IdTable.h"

class PercentileTrackerB : public IPercentileTracker{
public:

    PercentileTrackerB(std::vector<double> allValues);

    void insert(long long id, double value) override;
    void retract(long long id) override;
    double query(double p) override;

private:
    PercentileEngine engine;
    IdTable table;   
};