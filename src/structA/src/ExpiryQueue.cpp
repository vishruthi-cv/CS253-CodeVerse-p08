#include "ExpiryQueue.h"
#include <vector>

ExpiryQueue::ExpiryQueue(int capacity)
{
    buf.resize(capacity);
    head = 0;
    count = 0;
}

void ExpiryQueue::push(long long id)
{
    // Auto-grow the ring buffer if we need to hold retracted items alongside live ones
    if (count == (int)buf.size())
    {
        std::vector<long long> newBuf(buf.size() * 2 + 1);
        for (int i = 0; i < count; i++)
        {
            newBuf[i] = buf[(head + i) % buf.size()];
        }
        buf = newBuf;
        head = 0;
    }

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
