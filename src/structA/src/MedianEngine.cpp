#include "MedianEngine.h"
#include <cmath>
#include <vector>
#include <string>

static bool reportFail(std::string *why, const std::string &msg)
{
    if (why != nullptr)
        *why = msg;
    return false;
}

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

bool MedianEngine::checkConsistency(std::string *why) const
{
    const std::vector<item> &a = low.items();
    const std::vector<item> &b = high.items();

    // 1. Heap order: low is a max heap, high is a min heap
    for (int i = 1; i < (int)a.size(); i++)
    {
        int parent = (i - 1) / 2;
        if (itemLess(a[parent], a[i]))
            return reportFail(why, "low heap order broken at index " + std::to_string(i));
    }
    for (int i = 1; i < (int)b.size(); i++)
    {
        int parent = (i - 1) / 2;
        if (itemLess(b[i], b[parent]))
            return reportFail(why, "high heap order broken at index " + std::to_string(i));
    }

    // 2. Walk every item physically inside the heaps and compare with its table record
    int liveLowSeen = 0, liveHighSeen = 0, deadSeen = 0;
    bool haveLow = false, haveHigh = false;
    item maxLiveLow, minLiveHigh;

    for (int i = 0; i < (int)a.size(); i++)
    {
        const entry *e = table.find(a[i].id);
        if (e == nullptr)
            return reportFail(why, "item in low heap has no table record, id " + std::to_string(a[i].id));
        if (e->side != 0)
            return reportFail(why, "table says high but item is in low heap, id " + std::to_string(a[i].id));
        if (e->dead)
            deadSeen++;
        else
        {
            liveLowSeen++;
            if (!haveLow || itemLess(maxLiveLow, a[i]))
            {
                maxLiveLow = a[i];
                haveLow = true;
            }
        }
    }
    for (int i = 0; i < (int)b.size(); i++)
    {
        const entry *e = table.find(b[i].id);
        if (e == nullptr)
            return reportFail(why, "item in high heap has no table record, id " + std::to_string(b[i].id));
        if (e->side != 1)
            return reportFail(why, "table says low but item is in high heap, id " + std::to_string(b[i].id));
        if (e->dead)
            deadSeen++;
        else
        {
            liveHighSeen++;
            if (!haveHigh || itemLess(b[i], minLiveHigh))
            {
                minLiveHigh = b[i];
                haveHigh = true;
            }
        }
    }

    // 3. Counters must match what we actually counted
    if (liveLowSeen != lowLive)
        return reportFail(why, "lowLive is wrong");
    if (liveHighSeen != highLive)
        return reportFail(why, "highLive is wrong");
    if (deadSeen != deadItems)
        return reportFail(why, "deadItems is wrong");

    // 4. Table has exactly one record per physical item (no extras, no duplicate ids)
    if (table.size() != (int)(a.size() + b.size()))
        return reportFail(why, "table size does not match physical heap size");

    // 5. low holds exactly the right number of live items for this percentile
    if (lowLive != targetLow())
        return reportFail(why, "lowLive does not equal targetLow()");

    // 6. Every live item in low is at most every live item in high
    if (haveLow && haveHigh && itemLess(minLiveHigh, maxLiveLow))
        return reportFail(why, "a live item in high is smaller than a live item in low");

    // 7. Both tops are live, so query() reads a real value
    if (!a.empty())
    {
        const entry *e = table.find(a[0].id);
        if (e != nullptr && e->dead)
            return reportFail(why, "top of low heap is dead");
    }
    if (!b.empty())
    {
        const entry *e = table.find(b[0].id);
        if (e != nullptr && e->dead)
            return reportFail(why, "top of high heap is dead");
    }

    // 8. Memory bound from the purge rule, and the queue never exceeds the window
    if (deadItems > lowLive + highLive)
        return reportFail(why, "dead items outnumber live items, purge missed");
    if (queue.size() > window)
        return reportFail(why, "expiry queue is larger than the window");

    return true;
}