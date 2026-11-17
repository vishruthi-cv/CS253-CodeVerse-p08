// bench/bench.cpp  --  Benchmark harness for Project 8 (Analyst)
//
// Two commands:
//
//   bench check <w> <U> <r%> <p%> <N> <seed>
//       Feeds the same stream to A and B and compares them at every step.
//       Run this first: timing a structure that gives wrong answers is useless.
//
//   bench run <exp> <A|B> <w> <U> <r%> <p%> <N> <seed> <reps> <csv>
//       Times one structure. Rep 0 is a warm-up and is also used to measure
//       memory. Reps 1..reps are the timed runs. One CSV row is appended per rep.
//
// Symbols:  w = window size,  U = number of distinct values,
//           r% = chance (in percent) that a step also retracts an earlier reading,
//           p% = percentile (50 = median),  N = number of readings (steps),
//           seed = random seed (rep k uses seed + k, so every run is reproducible).
//
// One "step" = insert a reading, maybe retract one earlier reading, then query.
// Expiry is part of the insert: A expires inside add(), B's adapter expires the
// reading that leaves the window (the same rule, so A and B see the same window).

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "MedianEngine.h"      // Structure A  (Struct_A/include)
#include "PercentileTrackerB.h"  // Structure B  (Struct_B/include; needs Application/include)

// B's getPercentile(p): does it expect a fraction (0.5 = median) or a percent (50 = median)?
// 0 = fraction (what the check mode confirmed so far), 1 = percent.
// If "bench check" says DISAGREE on every line, flip this to 1.
#ifndef B_P_IS_PERCENT
#define B_P_IS_PERCENT 0
#endif

// ---------------------------------------------------------------- memory ----
// Resident memory of this process right now, in KB.
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
static long long rssKB() {
    PROCESS_MEMORY_COUNTERS pmc;
    std::memset(&pmc, 0, sizeof(pmc));
    pmc.cb = sizeof(pmc);
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) return 0;
    return (long long)(pmc.WorkingSetSize / 1024);
}
#else
static long long rssKB() {
    std::ifstream f("/proc/self/status");
    std::string line;
    while (std::getline(f, line))
        if (line.compare(0, 6, "VmRSS:") == 0) return std::atoll(line.c_str() + 6);
    return -1;
}
#endif

static volatile double g_sink;  // stops the compiler from deleting the queries

struct Counters {
    long long purges = 0;
    long long maxPhysical = 0;  // largest number of entries physically in A's heaps
    long long deadEnd = 0;      // dead entries inside A's heaps at the end
};

// ------------------------------------------------------------- adapters ----
// A thin wrapper per structure so the harness calls both in the same way.
// Ids are 0, 1, 2, ... in insertion order, which is also how A numbers them.

struct TrackerA {
    MedianEngine eng;
    bool idOk = true;
    TrackerA(int w, int pct) : eng(w, pct / 100.0) {}
    void insert(long long id, long long v) {
        if (eng.add((double)v) != id) idOk = false;  // A expires the oldest itself
    }
    void retract(long long id) { eng.retract(id); }
    bool empty() const { return eng.empty(); }
    long long live() const { return eng.liveCount(); }
    double query() const { return eng.query(); }
    void sample(Counters& c) const {
        c.purges = eng.purgeCount();
        c.deadEnd = eng.deadCount();
        if (eng.physicalSize() > c.maxPhysical) c.maxPhysical = eng.physicalSize();
    }
};

struct TrackerB {
    PercentileTrackerB tr;
    int w;
    double p;
    std::vector<char> alive;  // is the reading with id (slot + k*w) still in B? indexed by id % w
    long long liveN = 0;
    TrackerB(int w_, int pct_, long long U) : tr(domain(U)), w(w_), p(B_P_IS_PERCENT ? (double)pct_ : pct_ / 100.0), alive(w_, 0) {}
    static std::vector<double> domain(long long U) {
        std::vector<double> d((size_t)U);
        for (long long i = 0; i < U; i++) d[(size_t)i] = (double)i;
        return d;
    }
    void insert(long long id, long long v) {
        int slot = (int)(id % w);
        if (alive[slot]) {  // this slot still holds id - w, which now leaves the window
            tr.erase(id - w);
            alive[slot] = 0;
            liveN--;
        }
        tr.insert(id, (double)v);
        alive[slot] = 1;
        liveN++;
    }
    void retract(long long id) {
        int slot = (int)(id % w);
        if (alive[slot]) {
            tr.erase(id);
            alive[slot] = 0;
            liveN--;
        }
    }
    bool empty() const { return liveN == 0; }
    long long live() const { return liveN; }
    double query() { return tr.getPercentile(p); }
    void sample(Counters&) const {}
};

// ----------------------------------------------------------------- stream ----
// One step of the stream, applied to every tracker passed in. The random numbers
// are drawn here, so A and B receive exactly the same readings and retractions.
template <class... Ts>
inline void step(long long n, std::mt19937_64& rng, int w, long long U, int rPct,
                 std::vector<char>& alive, long long& retracts, Ts&... ts) {
    long long v = (long long)(rng() % (unsigned long long)U);
    (ts.insert(n, v), ...);
    alive[n % w] = 1;
    if ((int)(rng() % 100) < rPct) {
        long long span = (n + 1 < w) ? n + 1 : w;                  // ids still in the window
        long long id = n - (long long)(rng() % (unsigned long long)span);
        if (alive[id % w]) {
            (ts.retract(id), ...);
            alive[id % w] = 0;
            retracts++;
        }
    }
}

template <class T>
double timedLoop(T& t, int w, long long U, int rPct, long long N, std::mt19937_64& rng,
                 std::vector<char>& alive, long long& retracts, Counters& c) {
    double sink = 0;
    auto t0 = std::chrono::steady_clock::now();
    for (long long n = 0; n < N; n++) {
        step(n, rng, w, U, rPct, alive, retracts, t);
        if (!t.empty()) sink += t.query();
        if ((n & 1023) == 0) t.sample(c);
    }
    auto t1 = std::chrono::steady_clock::now();
    t.sample(c);
    g_sink = sink;
    return std::chrono::duration<double>(t1 - t0).count();
}

template <class Factory>
void runConfig(const std::string& exp, const std::string& name, int w, long long U, int rPct,
               int pct, long long N, unsigned long long seed, int reps, const std::string& csv,
               Factory make) {
    bool fresh;
    {
        std::ifstream f(csv, std::ios::binary);
        fresh = (f.peek() == std::char_traits<char>::eof());
    }
    std::ofstream out(csv, std::ios::app);
    if (fresh)
        out << "exp,structure,w,U,r_pct,p_pct,N,seed,rep,seconds,ns_per_step,retracts,"
               "mem_kb,purges,max_physical,dead_end\n";

    std::vector<char> alive((size_t)w, 0);  // harness bookkeeping, same for A and B
    for (int rep = 0; rep <= reps; rep++) {
        std::fill(alive.begin(), alive.end(), 0);
        long long before = (rep == 0) ? rssKB() : -1;
        auto t = make();
        Counters c;
        long long retracts = 0;
        unsigned long long s = seed + (unsigned long long)rep;
        std::mt19937_64 rng(s);
        double sec = timedLoop(*t, w, U, rPct, N, rng, alive, retracts, c);
        long long mem = (rep == 0) ? rssKB() - before : -1;  // memory is measured on rep 0
        double ns = sec * 1e9 / (double)N;
        out << exp << ',' << name << ',' << w << ',' << U << ',' << rPct << ',' << pct << ',' << N
            << ',' << s << ',' << rep << ',' << sec << ',' << ns << ',' << retracts << ',' << mem
            << ',' << c.purges << ',' << c.maxPhysical << ',' << c.deadEnd << '\n';
        out.flush();
        std::printf("%s %s w=%d U=%lld r=%d%% rep %d%s: %.3f s  (%.1f ns/step)  mem=%lld KB\n",
                    exp.c_str(), name.c_str(), w, U, rPct, rep, rep == 0 ? " (warm-up)" : "", sec,
                    ns, mem);
    }
}

// ------------------------------------------------------------------ check ----
static int checkMode(int w, long long U, int rPct, int pct, long long N,
                     unsigned long long seed) {
    TrackerA a(w, pct);
    TrackerB b(w, pct, U);
    std::vector<char> alive((size_t)w, 0);
    std::mt19937_64 rng(seed);
    long long retracts = 0, bad = 0;
    for (long long n = 0; n < N; n++) {
        step(n, rng, w, U, rPct, alive, retracts, a, b);
        bool ea = a.empty(), eb = b.empty();
        bool ok = (ea == eb) && (a.live() == b.live()) && (ea || a.query() == b.query());
        if (!ok) {
            if (bad < 5)
                std::printf("MISMATCH at step %lld: live A=%lld B=%lld, query A=%g B=%g\n", n,
                            a.live(), b.live(), ea ? 0.0 : a.query(), eb ? 0.0 : b.query());
            bad++;
        }
    }
    if (!a.idOk) {
        std::printf("PROBLEM: A returned an id that is not 0,1,2,... in insertion order\n");
        bad++;
    }
    std::printf("check: w=%d U=%lld r=%d%% p=%d%% N=%lld seed=%llu retractions=%lld -> %s\n", w, U,
                rPct, pct, N, seed, retracts, bad == 0 ? "A and B AGREE" : "DISAGREE");
    return bad == 0 ? 0 : 1;
}

static int usage() {
    std::printf(
        "usage:\n"
        "  bench check <w> <U> <r%%> <p%%> <N> <seed>\n"
        "  bench run <exp> <A|B> <w> <U> <r%%> <p%%> <N> <seed> <reps> <csv>\n");
    return 2;
}

int main(int argc, char** argv) {
    if (argc == 8 && std::strcmp(argv[1], "check") == 0) {
        int w = std::atoi(argv[2]);
        long long U = std::atoll(argv[3]);
        int r = std::atoi(argv[4]), p = std::atoi(argv[5]);
        long long N = std::atoll(argv[6]);
        unsigned long long seed = std::strtoull(argv[7], nullptr, 10);
        if (w < 1 || U < 1 || r < 0 || r > 100 || p < 1 || p > 100 || N < 1) return usage();
        return checkMode(w, U, r, p, N, seed);
    }
    if (argc == 12 && std::strcmp(argv[1], "run") == 0) {
        std::string exp = argv[2], name = argv[3];
        int w = std::atoi(argv[4]);
        long long U = std::atoll(argv[5]);
        int r = std::atoi(argv[6]), p = std::atoi(argv[7]);
        long long N = std::atoll(argv[8]);
        unsigned long long seed = std::strtoull(argv[9], nullptr, 10);
        int reps = std::atoi(argv[10]);
        std::string csv = argv[11];
        if (w < 1 || U < 1 || r < 0 || r > 100 || p < 1 || p > 100 || N < 1 || reps < 1 ||
            (name != "A" && name != "B"))
            return usage();
        if (name == "A")
            runConfig(exp, name, w, U, r, p, N, seed, reps, csv,
                      [&] { return std::make_unique<TrackerA>(w, p); });
        else
            runConfig(exp, name, w, U, r, p, N, seed, reps, csv,
                      [&] { return std::make_unique<TrackerB>(w, p, U); });
        return 0;
    }
    return usage();
}