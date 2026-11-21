#include <string>
#include "../include/MinHeap.h"
#include <string>

static bool reportFail(std::string *why, const std::string &msg)
{
    if (why != nullptr)
        *why = msg;
    return false;
}
void MinHeap::push(item x)
{
    a.push_back(x);
    int i = (int)a.size() - 1;
    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (itemLess(a[i], a[parent]))
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

item MinHeap::pop()
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

item MinHeap::top() const
{
    return a[0];
}

int MinHeap::size() const
{
    return (int)a.size();
}

bool MinHeap::empty() const
{
    return a.empty();
}

const std::vector<item> &MinHeap::items() const
{
    return a;
}

void MinHeap::build(std::vector<item> list)
{
    a = list;
    for (int i = (int)a.size() / 2 - 1; i >= 0; i--)
    {
        siftDown(i);
    }
}

void MinHeap::siftDown(int i)
{
    int n = a.size();

    while (2 * i + 1 < n)
    {
        int child = 2 * i + 1;

        if (child + 1 < n && itemLess(a[child + 1], a[child]))
            child++;

        if (!itemLess(a[child], a[i]))
            break;

        item temp = a[i];
        a[i] = a[child];
        a[child] = temp;

        i = child;
    }
}
