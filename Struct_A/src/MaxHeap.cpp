#include "MaxHeap.h"

void MaxHeap::push(item x)
{
    a.push_back(x);
    int i = (int)a.size() - 1;
    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (itemLess(a[parent], a[i]))
        {
            item temp = a[parent];
            a[parent] = a[i];
            a[i] = temp;
            i = parent;
        }
        else
        {
            break;
        }
    }
}

item MaxHeap::pop()
{
    item result = a[0];
    a[0] = a.back();
    a.pop_back();
    if (!a.empty())
    {
        siftDown(0);
    }
    return result;
}

item MaxHeap::top() const
{
    return a[0];
}

int MaxHeap::size() const
{
    return (int)a.size();
}

bool MaxHeap::empty() const
{
    return a.empty();
}

const std::vector<item> &MaxHeap::items() const
{
    return a;
}
void MaxHeap::build(std::vector<item> list)
{
    a = list;
    for (int i = (int)a.size() / 2 - 1; i >= 0; i--)
    {
        siftDown(i);
    }
}

void MaxHeap::siftDown(int i)
{
    int n = (int)a.size();
    while (true)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int biggest = i;
        if (left < n && itemLess(a[biggest], a[left]))
        {
            biggest = left;
        }
        if (right < n && itemLess(a[biggest], a[right]))
        {
            biggest = right;
        }
        if (biggest == i)
        {
            break;
        }
        item temp = a[i];
        a[i] = a[biggest];
        a[biggest] = temp;
        i = biggest;
    }
}