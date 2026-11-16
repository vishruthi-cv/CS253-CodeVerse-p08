#pragma once
#include <vector>
#include "SortedValueTable.h"
#include "FenwickTree.h"

class PercentileEngine
{
public:
    PercentileEngine(std::vector<double> allValues);
    void insert(double v);
    void remove(double v);
    int size() const;
    double kth(int k) const;
    double percentile(double p) const;   
    double median() const;

private:
    SortedValueTable table;
    FenwickTree fen;
    int total;
};