#include "PercentileTrackerB.h"

PercentileTrackerB::PercentileTrackerB(std::vector<double> allValues)
    : values(allValues), engine(allValues), table(100003)
{
}

void PercentileTrackerB::insert(int64_t id, double value){
    double old;
    if (table.get(id, old))
        return;                  
    table.put(id, value);
    engine.insert(value);
}

bool PercentileTrackerB::erase(int64_t id){
    double value;
    if (!table.get(id, value))
        return false;            
    engine.remove(value);
    table.erase(id);
    return true;
}

bool PercentileTrackerB::update(int64_t id, double newValue){
    double oldValue;
    if (!table.get(id, oldValue))
        return false;            
    engine.remove(oldValue);
    engine.insert(newValue);
    table.erase(id);
    table.put(id, newValue);
    return true;
}

double PercentileTrackerB::getMedian() const{
    int n = engine.size();
    if (n == 0)
        return -1.0;             
    if (n % 2 == 1)
        return engine.kth((n + 1) / 2);
    return (engine.kth(n / 2) + engine.kth(n / 2 + 1)) / 2.0;
}

double PercentileTrackerB::getPercentile(double p) const{
    if (engine.size() == 0)
        return -1.0;
    return engine.percentile(p / 100.0);   
}

size_t PercentileTrackerB::size() const{
    return engine.size();
}

bool PercentileTrackerB::empty() const{
    return engine.size() == 0;
}

void PercentileTrackerB::clear(){
    engine = PercentileEngine(values);
    table = IdTable(100003);
}

std::string PercentileTrackerB::getEngineName() const{
    return "StructB-Fenwick";
}