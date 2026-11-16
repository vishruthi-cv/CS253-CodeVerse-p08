#pragma once
// Adversarial input generators for Project 8 (running median with retractions).
//
// This file only makes test data, so library containers are allowed here.
// It does not use or replace Structure A or Structure B.
//
// Id rule: the k-th add in the stream gets id k (counting from 0).
// This matches MedianEngine, where nextId starts at 0 and goes up by one per add.

#include <vector>
#include <deque>
#include <random>
#include <algorithm>
#include <cstdio>

struct Op
{
    bool isRetract; // false = add, true = retract
    double value;   // used when isRetract is false
    long long id;   // used when isRetract is true
};

// -------------------------------------------------------------
// TARGET 1 (hurts Structure A): many dead entries stuck inside the heaps
// -------------------------------------------------------------

// Values only go up. The oldest reading is always the smallest live one.
// It sits deep inside the lower heap (a leaf), so when it expires it is a
// dead entry that never reaches the top. No retractions are needed.
inline std::vector<Op> genAscending(long long n)
{
    std::vector<Op> ops;
    ops.reserve(n);
    for (long long i = 0; i < n; i++)
    {
        ops.push_back({false, (double)(i + 1), 0});
    }
    return ops;
}

// Mirror image: values only go down. The oldest reading is the biggest one,
// so it expires as a dead leaf inside the upper heap.
inline std::vector<Op> genDescending(long long n)
{
    std::vector<Op> ops;
    ops.reserve(n);
    for (long long i = 0; i < n; i++)
    {
        ops.push_back({false, (double)(n - i), 0});
    }
    return ops;
}

// Some readings are "victims" with extreme values (very small or very big).
// Extreme values sit at the bottom of their heap, far from the top.
// We retract victims before they expire, so the dead entries stay buried.
//   n    = total number of operations (adds plus retracts)
//   w    = window size (must be the same w given to the engine)
//   rate = fraction of operations that are retractions (0 to 0.9)
inline std::vector<Op> genRetractExtremes(long long n, int w, double rate, unsigned long long seed)
{
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> coin(0.0, 1.0);
    std::uniform_real_distribution<double> middle(0.0, 1000000.0);

    if (rate < 0.0)
        rate = 0.0;
    if (rate > 0.9)
        rate = 0.9;

    // To retract once per 1/rate operations we need enough victims.
    double victimChance = rate / (1.0 - rate);
    if (victimChance > 1.0)
        victimChance = 1.0;

    std::vector<Op> ops;
    ops.reserve(n);
    std::deque<long long> victims; // ids of victims not yet retracted
    long long nextId = 0;
    long long lowCount = 0;
    long long highCount = 0;

    while ((long long)ops.size() < n)
    {
        // forget victims that already expired from the window
        while (!victims.empty() && victims.front() < nextId - w)
        {
            victims.pop_front();
        }

        if (!victims.empty() && coin(rng) < rate)
        {
            ops.push_back({true, 0.0, victims.front()});
            victims.pop_front();
            continue;
        }

        double v;
        if (coin(rng) < victimChance)
        {
            // alternate between a very small and a very big victim
            if (((lowCount + highCount) & 1) == 0)
            {
                v = -1.0e9 + (double)lowCount;
                lowCount++;
            }
            else
            {
                v = 1.0e9 + (double)highCount;
                highCount++;
            }
            victims.push_back(nextId);
        }
        else
        {
            v = middle(rng); // ordinary reading near the middle
        }
        ops.push_back({false, v, 0});
        nextId++;
    }
    return ops;
}

// -------------------------------------------------------------
// TARGET 2 (hurts Structure B): compressed domain much bigger than live count
// -------------------------------------------------------------

// Every add has a different value (a random shuffle of 1 to n), so the number
// of distinct values U in the whole stream is as big as it can be, while only
// w readings are live at a time. Optional random retractions of recent ids.
inline std::vector<Op> genAllDistinct(long long n, int w, double rate, unsigned long long seed)
{
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> coin(0.0, 1.0);

    std::vector<double> values(n);
    for (long long i = 0; i < n; i++)
    {
        values[i] = (double)(i + 1);
    }
    std::shuffle(values.begin(), values.end(), rng);

    std::vector<Op> ops;
    ops.reserve(n);
    long long nextId = 0;
    long long used = 0;

    while ((long long)ops.size() < n)
    {
        if (nextId > 0 && coin(rng) < rate)
        {
            long long lo = nextId - w;
            if (lo < 0)
                lo = 0;
            std::uniform_int_distribution<long long> pick(lo, nextId - 1);
            ops.push_back({true, 0.0, pick(rng)});
            continue;
        }
        ops.push_back({false, values[used], 0});
        used++;
        nextId++;
    }
    return ops;
}

// Friendly control for Structure B: only a few distinct values (U is tiny).
// Compare this with genAllDistinct to see the effect of a large U.
inline std::vector<Op> genFewDistinct(long long n, int w, double rate, unsigned long long seed, int distinct)
{
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> coin(0.0, 1.0);
    if (distinct < 1)
        distinct = 1;
    std::uniform_int_distribution<int> val(0, distinct - 1);

    std::vector<Op> ops;
    ops.reserve(n);
    long long nextId = 0;

    while ((long long)ops.size() < n)
    {
        if (nextId > 0 && coin(rng) < rate)
        {
            long long lo = nextId - w;
            if (lo < 0)
                lo = 0;
            std::uniform_int_distribution<long long> pick(lo, nextId - 1);
            ops.push_back({true, 0.0, pick(rng)});
            continue;
        }
        ops.push_back({false, (double)val(rng), 0});
        nextId++;
    }
    return ops;
}

// -------------------------------------------------------------
// Writing the operations to a text file
// -------------------------------------------------------------
// One operation per line:   add 12.5   or   retract 7
// If the instructor publishes a different input format, change only this function.
inline void writeOps(const std::vector<Op> &ops, FILE *out)
{
    for (size_t i = 0; i < ops.size(); i++)
    {
        if (ops[i].isRetract)
        {
            fprintf(out, "retract %lld\n", ops[i].id);
        }
        else
        {
            fprintf(out, "add %.6f\n", ops[i].value);
        }
    }
}