#pragma once
#include <vector>
#include "SortedValueTable.h"
#include "FenwickTree.h"

class PercentileEngine
{
public:
    PercentileEngine(std::vector<long long> allValues);
    void insert(long long v);
    void remove(long long v);
    int size() const;
    long long kth(int k) const;
    long long percentile(int p) const;
    long long median() const;

private:
    SortedValueTable table;
    FenwickTree fen;
    int total;
};