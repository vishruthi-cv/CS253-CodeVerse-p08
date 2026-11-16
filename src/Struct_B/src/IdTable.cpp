#include "IdTable.h"

IdTable::IdTable(int bucketCount){
    buckets.resize(bucketCount);
    count = 0;
}

int IdTable::bucketOf(long long id) const{
    long long n = (long long)buckets.size();
    return (int)(((id % n) + n) % n);
}

bool IdTable::get(long long id, double &value) const{
    const std::vector<Entry> &chain = buckets[bucketOf(id)];
    for (int i = 0; i < (int)chain.size(); i++){
        if (chain[i].id == id){
            value = chain[i].value;
            return true;
        }
    }
    return false;
}

void IdTable::put(long long id, double value){
    Entry e;
    e.id = id;
    e.value = value;
    buckets[bucketOf(id)].push_back(e);
    count++;
}

void IdTable::erase(long long id){
    std::vector<Entry> &chain = buckets[bucketOf(id)];
    for (int i = 0; i < (int)chain.size(); i++){
        if (chain[i].id == id){
            chain[i] = chain.back();   
            chain.pop_back();
            count--;
            return;
        }
    }
}

int IdTable::size() const{
    return count;
}