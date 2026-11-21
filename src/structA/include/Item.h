#pragma once
struct item
{
    long long id;
    double value;
};

inline bool
itemLess(const item &a, const item &b)
{

    if (a.value < b.value)
        return true;

    if (a.value == b.value)
    {

        if (a.id < b.id)
        {
            return true;
        }
    }

    return false;
}