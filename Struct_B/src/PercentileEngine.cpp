#include "PercentileEngine.h"
#include <cmath>

PercentileEngine::PercentileEngine(std::vector<double> allValues)
    : table(allValues), fen(table.count())
{
    total = 0;
}

void PercentileEngine::insert(double v)
{
    fen.add(table.positionOf(v), 1);
    total++;
}

void PercentileEngine::remove(double v)
{
    fen.add(table.positionOf(v), -1);
    total--;
}

int PercentileEngine::size() const
{
    return total;
}

double PercentileEngine::kth(int k) const
{
    return table.valueAt(fen.findKth(k));
}

double PercentileEngine::percentile(double p) const
{
    int k = (int)ceil(p * total - 0.000000001);
    if (k < 1)
        k = 1;
    if (k > total)
        k = total;
    return kth(k);
}

// lower median when the size is even
double PercentileEngine::median() const
{
    return kth((total + 1) / 2);
}