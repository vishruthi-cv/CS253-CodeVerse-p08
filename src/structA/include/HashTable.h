#pragma once
#include <vector>

struct entry
{
    long long key;
    int side = 0; // 0-lower,1-higher
    bool dead;    // true means waiting to be removed
};

// Hashtable written from scratch
class HashTable
{
public:
    HashTable();
    void insert(long long key, int side);
    entry *find(long long key);
    void erase(long long key);
    int size() const;
    const entry *find(long long key) const;

private:
    std::vector<std::vector<entry>> buckets;
    int count;
    int indexOf(long long key) const;
    void grow();
};