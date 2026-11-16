// Command line tool: writes an adversarial operation stream to standard output.
//
// Usage:  gen_adversary MODE N W RATE SEED
//   MODE : ascending | descending | retract_extremes | all_distinct | few_distinct
//   N    : total number of operations
//   W    : window size
//   RATE : fraction of operations that are retractions (0.0 to 0.9)
//   SEED : random seed (write it in your results log so runs can be repeated)
//
// Example: ./bin/gen_adversary ascending 1000000 100000 0 1 > ascending.txt

#include "adversary.h"
#include <cstdio>
#include <cstdlib>
#include <string>

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        fprintf(stderr, "usage: gen_adversary MODE N W RATE SEED\n");
        fprintf(stderr, "MODE: ascending | descending | retract_extremes | all_distinct | few_distinct\n");
        return 1;
    }

    std::string mode = argv[1];
    long long n = atoll(argv[2]);
    int w = atoi(argv[3]);
    double rate = atof(argv[4]);
    unsigned long long seed = strtoull(argv[5], nullptr, 10);

    std::vector<Op> ops;
    if (mode == "ascending")
        ops = genAscending(n);
    else if (mode == "descending")
        ops = genDescending(n);
    else if (mode == "retract_extremes")
        ops = genRetractExtremes(n, w, rate, seed);
    else if (mode == "all_distinct")
        ops = genAllDistinct(n, w, rate, seed);
    else if (mode == "few_distinct")
        ops = genFewDistinct(n, w, rate, seed, 50);
    else
    {
        fprintf(stderr, "unknown mode: %s\n", mode.c_str());
        return 1;
    }

    // settings go to the error stream so the data stream stays clean
    fprintf(stderr, "mode=%s n=%lld w=%d rate=%.3f seed=%llu ops=%zu\n",
            mode.c_str(), n, w, rate, seed, ops.size());

    writeOps(ops, stdout);
    return 0;
}