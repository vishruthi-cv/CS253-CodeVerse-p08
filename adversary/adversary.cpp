// adversary/adversary.cpp  --  Adversarial input generators for Project 8 (Adversary role)
//
// Build:  g++ -std=c++17 -O2 adversary/adversary.cpp -o build/adversary
// Usage:  build/adversary <pattern> <w> <U> <N> <seed> > ops.txt
//
//   pattern  deadheap  retract readings that sit deep inside A's heaps (extreme values),
//                      so lazy deletion can never prune them from a heap top
//            monotone  strictly increasing values and no retractions: every expiring
//                      reading is the smallest one, which sits at the bottom of A's lower heap
//            bigdomain every value distinct, U large, w small: B's compressed domain
//                      (size U) is huge compared with its live count (at most w)
//   w = window size, U = number of distinct values (values are 0..U-1),
//   N = number of readings, seed = random seed (same arguments give the same file).
//
// Output (text, one operation per line, ids are 0,1,2,... in insertion order):
//   # <comment line with the arguments>
//   I <value>   insert a reading (its id is the number of earlier I lines)
//   R <id>      retract the reading with this id (always a reading still in the window)
//   Q           query the median (after every insert and its optional retraction)
// Expiry is not written: a reading leaves the window when w newer readings have arrived.
//
// Large outputs are generated from the seed and never committed (see .gitignore).

#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 6) {
        std::fprintf(stderr, "usage: %s <deadheap|monotone|bigdomain> <w> <U> <N> <seed>\n", argv[0]);
        return 2;
    }
    std::string pat = argv[1];
    long long w = std::atoll(argv[2]), U = std::atoll(argv[3]), N = std::atoll(argv[4]);
    unsigned long long seed = std::strtoull(argv[5], nullptr, 10);
    if (w < 1 || U < 2 || N < 1 || (pat != "deadheap" && pat != "monotone" && pat != "bigdomain")) {
        std::fprintf(stderr, "bad arguments\n");
        return 2;
    }
    std::mt19937_64 rng(seed);
    std::printf("# adversary %s w=%lld U=%lld N=%lld seed=%llu\n", pat.c_str(), w, U, N, seed);

    std::vector<long long> value(N);      // value of each id
    std::vector<char> retracted(N, 0);
    std::vector<long long> decoys;        // ids of extreme readings that may still be live
    long long tail = U / 10 > 0 ? U / 10 : 1;

    for (long long n = 0; n < N; n++) {
        long long v;
        if (pat == "deadheap") {
            // Half the readings are decoys in the two tails (bottom or top of the value range),
            // the other half stay near the middle, so the median is stable and decoys sit deep.
            bool decoy = (rng() & 1) != 0;
            if (decoy) {
                long long off = (long long)(rng() % (unsigned long long)tail);
                v = (rng() & 1) ? off : U - 1 - off;
                decoys.push_back(n);
            } else {
                v = U / 2 - U / 20 + (long long)(rng() % (unsigned long long)(U / 10 + 1));
                if (v < 0) v = 0;
                if (v >= U) v = U - 1;
            }
        } else if (pat == "monotone") {
            v = (n * U) / N;                        // 0 .. U-1, non-decreasing
        } else {                                    // bigdomain: a permutation of 0..U-1 when N <= U
            v = (long long)(((unsigned long long)n * 2654435761ULL) % (unsigned long long)U);
        }
        value[n] = v;
        std::printf("I %lld\n", v);

        if (pat == "deadheap" && (rng() & 1)) {
            // retract a random decoy that is still in the window and not yet retracted
            while (!decoys.empty()) {
                size_t k = (size_t)(rng() % decoys.size());
                long long id = decoys[k];
                decoys[k] = decoys.back();
                decoys.pop_back();
                if (id > n - w && !retracted[id]) {  // id still in the window: ids n-w+1 .. n
                    retracted[id] = 1;
                    std::printf("R %lld\n", id);
                    break;
                }
            }
        }
        std::printf("Q\n");
    }
    return 0;
}