#include "PercentileEngine.h"

PercentileEngine::PercentileEngine(std::vector<long long> allValues) : table(allValues), fen(table.count()){
    total = 0;
}

void PercentileEngine::insert(long long v){
    fen.add(table.positionOf(v), 1);
    total++;
}

void PercentileEngine::remove(long long v){
    fen.add(table.positionOf(v), -1);
    total--;
}

int PercentileEngine::size() const{
    return total;
}

long long PercentileEngine::kth(int k) const{
    return table.valueAt(fen.findKth(k));
}

long long PercentileEngine::percentile(int p) const{
    int k = (p * total + 99) / 100;
    if (k < 1)
        k = 1;
    if (k > total)
        k = total;
    return kth(k);
}

long long PercentileEngine::median() const{
    return kth((total + 1) / 2);
}