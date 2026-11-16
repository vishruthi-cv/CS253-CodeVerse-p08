#include "ExpiryQueue.h"

ExpiryQueue::ExpiryQueue(int capacity)
{
    buf.resize(capacity);
    head = 0;
    count = 0;
}

void ExpiryQueue::push(long long id)
{
    int pos = (head + count) % (int)buf.size();
    buf[pos] = id;
    count++;
}

long long ExpiryQueue::pop()
{
    long long id = buf[head];
    head = (head + 1) % (int)buf.size();
    count--;
    return id;
}

int ExpiryQueue::size() const
{
    return count;
}

bool ExpiryQueue::full() const
{
    return count == (int)buf.size();
}