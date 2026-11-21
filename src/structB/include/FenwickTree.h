#pragma once
#include <vector>

class FenwickTree
{
public:
    FenwickTree(int n);
    void add(int pos, int delta);
    int findKth(int k) const;

private:
    std::vector<int> tree;
    int n;
    int topPower;
};