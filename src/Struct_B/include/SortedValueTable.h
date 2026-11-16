#pragma once
#include <vector>

class SortedValueTable
{
public:
    SortedValueTable(std::vector<double> list);
    int count() const;
    int positionOf(double v) const;
    double valueAt(int pos) const;

private:
    std::vector<double> a;
};