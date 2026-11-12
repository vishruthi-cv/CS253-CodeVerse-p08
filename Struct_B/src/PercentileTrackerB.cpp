#include "PercentileTrackerB.h"

PercentileTrackerB::PercentileTrackerB(std::vector<double> allValues)
    : engine(allValues)
{
}

void PercentileTrackerB::insert(long long id, double value)
{
    if (valueOf.count(id) > 0)
        return;                  
    valueOf[id] = value;
    engine.insert(value);
}

void PercentileTrackerB::retract(long long id)
{
    auto it = valueOf.find(id);
    if (it == valueOf.end())
        return;                  
    engine.remove(it->second);
    valueOf.erase(it);
}

double PercentileTrackerB::query(double p)
{
    if (engine.size() == 0)
        return 0;
    return engine.percentile(p);
}