#pragma once
#include "Item.h"
#include "MaxHeap.h"
#include "MinHeap.h"
#include "HashTable.h"
#include "ExpiryQueue.h"

class MedianEngine
{
public:
    MedianEngine(int windowSize, double percentile = 0.5);

    long long add(double value);
    bool retract(long long id);
    bool empty() const;
    double query() const;

    int liveCount() const;
    int deadCount() const;
    int physicalSize() const;
    int purgeCount() const;
    // Checks every internal rule. Returns true if all is well.
    // On failure returns false and, if why is not null, writes the reason.
    bool checkConsistency(std::string *why = nullptr) const;

private:
    int window;
    double p;
    long long nextId;
    MaxHeap low;
    MinHeap high;
    HashTable table;
    ExpiryQueue queue;
    int lowLive;
    int highLive;
    int deadItems;
    int purges;

    int targetLow() const;
    bool markDead(long long id);
    void cleanLow();
    void cleanHigh();
    void rebalance();
    void purgeIfNeeded();
    void finish();
};