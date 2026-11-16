#pragma once
#include <vector>

class IdTable{
public:
    IdTable(int bucketCount);
    bool get(long long id, double &value) const;   
    void put(long long id, double value);          
    void erase(long long id);                      
    int size() const;

private:
    struct Entry
    {
        long long id;
        double value;
    };
    std::vector<std::vector<Entry>> buckets;
    int count;

    int bucketOf(long long id) const;
};