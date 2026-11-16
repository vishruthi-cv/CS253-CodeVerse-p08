#pragma once
#include <vector>

class ExpiryQueue
{
public:
    ExpiryQueue(int capacity);
    void push(long long id);
    long long pop();
    int size() const;
    bool full() const;

private:
    std::vector<long long> buf;
    int head;
    int count;
};