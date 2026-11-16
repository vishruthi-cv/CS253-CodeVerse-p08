#include "PercentileTrackerB.h"

PercentileTrackerB::PercentileTrackerB(std::vector<double> allValues)
    : engine(allValues), table(100003)
{
}

void PercentileTrackerB::insert(long long id, double value){
    double old;
    if (table.get(id, old))
        return;                  
    table.put(id, value);
    engine.insert(value);
}

void PercentileTrackerB::retract(long long id){
    double value;
    if (!table.get(id, value))
        return;                  
    engine.remove(value);
    table.erase(id);
}

double PercentileTrackerB::query(double p){
    if (engine.size() == 0)
        return 0;
    return engine.percentile(p);
}