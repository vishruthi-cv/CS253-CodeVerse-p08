#pragma once
#include <vector>

class SortedValueTable
{
public:
    SortedValueTable(std::vector<long long> list);
    int count() const;
    int positionOf(long long v) const;
    long long valueAt(int pos) const;

private:
    std::vector<long long> a;
};