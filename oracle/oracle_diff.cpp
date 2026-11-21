#include "MedianEngine.h"
#include "PercentileTrackerB.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
const std::uint64_t kSeed = 20261121ULL;
const int kWindow = 1000;
const int kValueDomain = 10000;
const int kSteps = 1000000;
const int kRetractPercent = 25;
const int kCheckInterval = 256;

struct Reading { long long id; double value; };
struct Operation {
    enum Kind { Add, Retract } kind;
    long long token;
    double value;
};
struct Failure { std::size_t index; std::string reason; };

class SortedOracle {
public:
    void insert(long long id, double value) {
        std::size_t pos = 0;
        while (pos < readings.size() &&
               (readings[pos].value < value ||
                (readings[pos].value == value && readings[pos].id < id))) ++pos;
        readings.insert(readings.begin() + static_cast<std::ptrdiff_t>(pos), {id, value});
    }
    bool erase(long long id) {
        for (std::size_t i = 0; i < readings.size(); ++i) {
            if (readings[i].id == id) {
                readings.erase(readings.begin() + static_cast<std::ptrdiff_t>(i));
                return true;
            }
        }
        return false;
    }
    std::size_t size() const { return readings.size(); }
    double percentile(double p) const {
        const int rank = static_cast<int>(std::ceil(p * readings.size() - 1e-9));
        return readings[static_cast<std::size_t>(std::max(1, rank) - 1)].value;
    }
private:
    std::vector<Reading> readings;
};

Failure fail(std::size_t i, const std::string& why) { return {i, why}; }

bool runTrace(const std::vector<Operation>& trace, bool checkEveryOp, Failure& failure) {
    std::vector<double> domain;
    domain.reserve(kValueDomain);
    for (int v = 0; v < kValueDomain; ++v) domain.push_back(static_cast<double>(v));

    MedianEngine a50(kWindow, 0.5), a90(kWindow, 0.9);
    PercentileTrackerB b(domain);
    SortedOracle oracle;
    std::unordered_map<long long, long long> actualId;
    std::deque<long long> arrival;
    std::size_t adds = 0;

    for (std::size_t i = 0; i < trace.size(); ++i) {
        const Operation& op = trace[i];
        if (op.kind == Operation::Add) {
            if (arrival.size() == static_cast<std::size_t>(kWindow)) {
                const long long expired = arrival.front();
                arrival.pop_front();
                b.erase(expired);
                oracle.erase(expired);
            }
            const long long id50 = a50.add(op.value);
            const long long id90 = a90.add(op.value);
            if (id50 != id90) { failure = fail(i, "Structure A engines assigned different IDs"); return false; }
            actualId[op.token] = id50;
            arrival.push_back(id50);
            b.insert(id50, op.value);
            oracle.insert(id50, op.value);
            ++adds;
        } else {
            const auto found = actualId.find(op.token);
            if (found != actualId.end()) {
                const long long id = found->second;
                const bool ra = a50.retract(id);
                const bool ra90 = a90.retract(id);
                const bool rb = b.erase(id);
                const bool ro = oracle.erase(id);
                if (ra != ra90 || ra != rb || ra != ro) {
                    failure = fail(i, "retraction result disagreed across implementations"); return false;
                }
            }
        }

        if (a50.liveCount() != static_cast<int>(oracle.size()) ||
            a90.liveCount() != static_cast<int>(oracle.size()) ||
            b.size() != oracle.size()) {
            failure = fail(i, "live item counts differ"); return false;
        }
        if (oracle.size() != 0) {
            const double want50 = oracle.percentile(0.50), want90 = oracle.percentile(0.90);
            if (a50.query() != want50 || a90.query() != want90 ||
                b.getPercentile(50.0) != want50 || b.getPercentile(90.0) != want90) {
                failure = fail(i, "p50 or p90 differs from the sorted oracle"); return false;
            }
        } else if (!a50.empty() || !a90.empty() || !b.empty()) {
            failure = fail(i, "empty-state disagreement"); return false;
        }

        if (checkEveryOp || (adds != 0 && adds % kCheckInterval == 0 && op.kind == Operation::Add)) {
            std::string why;
            if (!a50.checkConsistency(&why)) { failure = fail(i, "A p50 checker: " + why); return false; }
            if (!a90.checkConsistency(&why)) { failure = fail(i, "A p90 checker: " + why); return false; }
        }
    }
    return true;
}

void shrinkFailure(std::vector<Operation>& trace) {
    std::size_t granularity = 2;
    while (trace.size() >= 2) {
        const std::size_t chunk = (trace.size() + granularity - 1) / granularity;
        bool reduced = false;
        for (std::size_t start = 0; start < trace.size(); start += chunk) {
            const std::size_t end = std::min(trace.size(), start + chunk);
            std::vector<Operation> candidate;
            candidate.reserve(trace.size() - (end - start));
            candidate.insert(candidate.end(), trace.begin(), trace.begin() + static_cast<std::ptrdiff_t>(start));
            candidate.insert(candidate.end(), trace.begin() + static_cast<std::ptrdiff_t>(end), trace.end());
            Failure ignored{};
            // During shrinking, check every operation so a small counterexample
            // cannot hide a brief invariant violation between sampled checks.
            if (!runTrace(candidate, true, ignored)) {
                trace.swap(candidate);
                granularity = std::max<std::size_t>(2, granularity - 1);
                reduced = true;
                break;
            }
        }
        if (!reduced) {
            if (granularity >= trace.size()) break;
            granularity = std::min(trace.size(), granularity * 2);
        }
    }
}

std::vector<Operation> generateTrace() {
    std::mt19937_64 rng(kSeed);
    std::vector<Operation> trace;
    trace.reserve(kSteps + kSteps / 3);
    std::vector<unsigned char> alive(kSteps, 0);
    for (int n = 0; n < kSteps; ++n) {
        const double value = static_cast<double>(rng() % kValueDomain);
        trace.push_back({Operation::Add, n, value});
        alive[static_cast<std::size_t>(n)] = 1;
        if (n >= kWindow) alive[static_cast<std::size_t>(n - kWindow)] = 0;

        if (rng() % 100 < kRetractPercent) {
            const int span = std::min(n + 1, kWindow);
            const int candidate = n - static_cast<int>(rng() % static_cast<std::uint64_t>(span));
            if (alive[static_cast<std::size_t>(candidate)]) {
                trace.push_back({Operation::Retract, candidate, 0.0});
                alive[static_cast<std::size_t>(candidate)] = 0;
            }
        }
    }
    return trace;
}
} // namespace

int main() {
    std::cout << "Seed: " << kSeed << "\nSteps: " << kSteps
              << "\nWindow: " << kWindow << "\n";
    const std::vector<Operation> trace = generateTrace();
    Failure failure{};
    if (!runTrace(trace, false, failure)) {
        std::cout << "FAIL at operation " << failure.index << ": " << failure.reason << "\n";
        std::vector<Operation> minimized(trace.begin(), trace.begin() + static_cast<std::ptrdiff_t>(failure.index + 1));
        shrinkFailure(minimized);
        std::cout << "Shrunk failing sequence (" << minimized.size() << " operations):\n";
        for (const Operation& op : minimized) {
            if (op.kind == Operation::Add) std::cout << "ADD " << op.token << " " << op.value << "\n";
            else std::cout << "RETRACT " << op.token << "\n";
        }
        return 1;
    }
    std::cout << "ALL TESTS PASSED\n";
    return 0;
}
