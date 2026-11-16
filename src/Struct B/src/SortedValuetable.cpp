#include "SortedValueTable.h"
#include <algorithm>

SortedValueTable::SortedValueTable(std::vector<double> list)
{
    std::sort(list.begin(), list.end());
    for (int i = 0; i < (int)list.size(); i++){
        if (i == 0 || list[i] != list[i - 1])
            a.push_back(list[i]);
    }
}

int SortedValueTable::count() const{
    return (int)a.size();
}

int SortedValueTable::positionOf(double v) const{
    int low = 0;
    int high = (int)a.size() - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        if (a[mid] == v)
            return mid + 1;
        if (a[mid] < v)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return 0;
}

double SortedValueTable::valueAt(int pos) const
{
    return a[pos - 1];
}