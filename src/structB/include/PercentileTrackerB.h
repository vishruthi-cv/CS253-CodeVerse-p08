#pragma once
#include <vector>
#include <string>
#include "IPercentileTracker.h"
#include "PercentileEngine.h"
#include "IdTable.h"

class PercentileTrackerB : public IPercentileTracker
{
public:
    PercentileTrackerB(std::vector<double> allValues);

    void insert(int64_t id, double value) override;
    bool erase(int64_t id) override;
    bool update(int64_t id, double newValue) override;

    double getMedian() const override;
    double getPercentile(double p) const override;   

    size_t size() const override;
    bool empty() const override;
    void clear() override;
    std::string getEngineName() const override;

private:
    std::vector<double> values;   
    PercentileEngine engine;
    IdTable table;
};