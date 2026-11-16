#include "MedianEngine.h"
#include <cmath>
#include <vector>

MedianEngine::MedianEngine(int windowSize, double percentile)
    : window(windowSize < 1 ? 1 : windowSize),
      p(percentile),
      nextId(0),
      queue(windowSize < 1 ? 1 : windowSize)
{
    lowLive = 0;
    highLive = 0;
    deadItems = 0;
    purges = 0;
    if (p <= 0.0)
        p = 0.000001;
    if (p > 1.0)
        p = 1.0;
}

// How many live items low should hold
int MedianEngine::targetLow() const
{
    int n = lowLive + highLive;
    if (n == 0)
        return 0;
    int k = (int)std::ceil(p * n - 0.000000001);
    if (k < 1)
        k = 1;
    if (k > n)
        k = n;
    return k;
}

// Lazy deletion: only mark the item as dead, do not touch the heap
bool MedianEngine::markDead(long long id)
{
    entry *e = table.find(id);
    if (e == nullptr || e->dead)
    {
        return false;
    }
    e->dead = true;
    if (e->side == 0)
        lowLive--;
    else
        highLive--;
    deadItems++;
    return true;
}

// Really remove dead items sitting on top of the low heap
void MedianEngine::cleanLow()
{
    while (!low.empty())
    {
        item t = low.top();
        entry *e = table.find(t.id);
        if (e != nullptr && e->dead)
        {
            low.pop();
            table.erase(t.id);
            deadItems--;
        }
        else
        {
            break;
        }
    }
}

// Really remove dead items sitting on top of the high heap
void MedianEngine::cleanHigh()
{
    while (!high.empty())
    {
        item t = high.top();
        entry *e = table.find(t.id);
        if (e != nullptr && e->dead)
        {
            high.pop();
            table.erase(t.id);
            deadItems--;
        }
        else
        {
            break;
        }
    }
}

// Move items between the heaps until low holds exactly k live items
void MedianEngine::rebalance()
{
    int target = targetLow();
    while (lowLive > target)
    {
        cleanLow();
        item it = low.pop();
        high.push(it);
        table.find(it.id)->side = 1;
        lowLive--;
        highLive++;
    }
    while (lowLive < target)
    {
        cleanHigh();
        item it = high.pop();
        low.push(it);
        table.find(it.id)->side = 0;
        lowLive++;
        highLive--;
    }
}

// If dead items outnumber live items, rebuild both heaps without the dead ones
void MedianEngine::purgeIfNeeded()
{
    if (deadItems <= lowLive + highLive)
    {
        return;
    }

    std::vector<item> keepLow;
    std::vector<item> keepHigh;

    const std::vector<item> &a = low.items();
    for (int i = 0; i < (int)a.size(); i++)
    {
        entry *e = table.find(a[i].id);
        if (e != nullptr && e->dead)
        {
            table.erase(a[i].id);
        }
        else
        {
            keepLow.push_back(a[i]);
        }
    }

    const std::vector<item> &b = high.items();
    for (int i = 0; i < (int)b.size(); i++)
    {
        entry *e = table.find(b[i].id);
        if (e != nullptr && e->dead)
        {
            table.erase(b[i].id);
        }
        else
        {
            keepHigh.push_back(b[i]);
        }
    }

    low.build(keepLow);
    high.build(keepHigh);
    deadItems = 0;
    purges++;
}

// Run after every add and retract
void MedianEngine::finish()
{
    rebalance();
    purgeIfNeeded();
    cleanLow();
    cleanHigh();
}

long long MedianEngine::add(double value)
{
    // Sliding window: when full, the oldest reading expires
    if (queue.full())
    {
        long long oldId = queue.pop();
        markDead(oldId); // does nothing if it was already retracted
    }

    long long id = nextId;
    nextId++;
    queue.push(id);

    item it;
    it.id = id;
    it.value = value;

    cleanLow();
    cleanHigh();

    bool goLow;
    if (lowLive > 0)
    {
        goLow = !itemLess(low.top(), it); // new item <= top of low
    }
    else if (highLive == 0)
    {
        goLow = true;
    }
    else
    {
        goLow = itemLess(it, high.top());
    }

    if (goLow)
    {
        low.push(it);
        table.insert(id, 0);
        lowLive++;
    }
    else
    {
        high.push(it);
        table.insert(id, 1);
        highLive++;
    }

    finish();
    return id;
}

bool MedianEngine::retract(long long id)
{
    if (!markDead(id))
    {
        return false;
    }
    finish();
    return true;
}

bool MedianEngine::empty() const
{
    return lowLive + highLive == 0;
}

double MedianEngine::query() const
{
    return low.top().value;
}

int MedianEngine::liveCount() const { return lowLive + highLive; }
int MedianEngine::deadCount() const { return deadItems; }
int MedianEngine::physicalSize() const { return low.size() + high.size(); }
int MedianEngine::purgeCount() const { return purges; }