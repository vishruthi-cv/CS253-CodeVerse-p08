#pragma once
#include <vector>
#include "Item.h"

class MaxHeap
{
public:
    void push(item x);
    item pop();
    item top() const;
    int size() const;
    bool empty() const;
    const std::vector<item> &items() const;
    void build(std::vector<item> list);

private:
    std::vector<item> a;
    void siftDown(int i);
};