#include "HashTable.h"

HashTable::HashTable()
{
    buckets.resize(16);
    count = 0;
}

int HashTable::indexOf(long long key) const
{
    long long n = buckets.size();
    long long m = key % n;
    if (m < 0)
    {
        m = m + n;
    }
    return (int)m;
}
int HashTable::size() const
{
    return count;
}

void HashTable::insert(long long key, int side)
{
    if (count > 2 * (int)buckets.size())
    {
        grow();
    }
    entry e;
    e.key = key;
    e.side = side;
    e.dead = false;
    buckets[indexOf(key)].push_back(e);
    count++;
}

entry *HashTable::find(long long key)
{
    std::vector<entry> &b = buckets[indexOf(key)];
    for (int i = 0; i < (int)b.size(); i++)
    {
        if (b[i].key == key)
        {
            return &b[i];
        }
    }
    return nullptr;
}

void HashTable::erase(long long key)
{
    std::vector<entry> &b = buckets[indexOf(key)];
    for (int i = 0; i < (int)b.size(); i++)
    {
        if (b[i].key == key)
        {
            b[i] = b.back();
            b.pop_back();
            count--;
            return;
        }
    }
}

void HashTable::grow()
{
    std::vector<std::vector<entry>> old = buckets;
    buckets.clear();
    buckets.resize(old.size() * 2);
    for (int i = 0; i < (int)old.size(); i++)
    {
        for (int j = 0; j < (int)old[i].size(); j++)
        {
            buckets[indexOf(old[i][j].key)].push_back(old[i][j]);
        }
    }
}