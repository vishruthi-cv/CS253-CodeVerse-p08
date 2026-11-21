#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "MedianEngine.h"
#include "PercentileTrackerB.h"

using namespace std;

struct Reading {
    long long id;
    double value;
};

class SortedOracle {
private:
    vector<Reading> readings;

public:
    void insert(long long id, double value) {
        int position = 0;

        while (position < static_cast<int>(readings.size()) &&
               (readings[position].value < value ||
                (readings[position].value == value &&
                 readings[position].id < id))) {
            position++;
        }

        readings.insert(readings.begin() + position, {id, value});
    }

    bool retract(long long id) {
        for (int i = 0; i < static_cast<int>(readings.size()); i++) {
            if (readings[i].id == id) {
                readings.erase(readings.begin() + i);
                return true;
            }
        }

        return false;
    }

    int size() const {
        return static_cast<int>(readings.size());
    }

    double query(double p) const {
        int rank = static_cast<int>(ceil(p * size() - 0.000000001));

        if (rank < 1) rank = 1;
        if (rank > size()) rank = size();

        return readings[rank - 1].value;
    }
};

bool compare(
    const string& step,
    const SortedOracle& oracle,
    MedianEngine& engineA50,
    MedianEngine& engineA90,
    PercentileTrackerB& trackerB
) {
    string why;

    if (!engineA50.checkConsistency(&why)) {
        cout << "Structure A p50 consistency check failed after "
             << step << ": " << why << '\n';
        return false;
    }

    why.clear();

    if (!engineA90.checkConsistency(&why)) {
        cout << "Structure A p90 consistency check failed after "
             << step << ": " << why << '\n';
        return false;
    }

    optional<double> b50Result = trackerB.getPercentile(50.0);
    optional<double> b90Result = trackerB.getPercentile(90.0);

    if (!b50Result.has_value() || !b90Result.has_value()) {
        cout << "Structure B returned no percentile after "
             << step << '\n';
        return false;
    }

    double expected50 = oracle.query(0.5);
    double expected90 = oracle.query(0.9);

    double a50 = engineA50.query();
    double a90 = engineA90.query();
    double b50 = b50Result.value();
    double b90 = b90Result.value();

    if (a50 != expected50 || a90 != expected90 ||
        b50 != expected50 || b90 != expected90 ||
        engineA50.liveCount() != oracle.size() ||
        engineA90.liveCount() != oracle.size() ||
        trackerB.size() != static_cast<size_t>(oracle.size())) {
        cout << "MISMATCH after " << step << '\n';
        cout << "Oracle:      p50 = " << expected50
             << ", p90 = " << expected90 << '\n';
        cout << "Structure A: p50 = " << a50
             << ", p90 = " << a90 << '\n';
        cout << "Structure B: p50 = " << b50
             << ", p90 = " << b90 << '\n';
        return false;
    }

    cout << "PASS after " << step
         << ": p50 = " << expected50
         << ", p90 = " << expected90 << '\n';

    return true;
}

int main() {
    const int windowSize = 5;
    const double data[] = {5, 1, 9, 3, 7, 8, 2, 8};
    const int dataCount = 8;

    // Structure B needs all possible values before the test begins.
    vector<double> allValues(data, data + dataCount);

    MedianEngine engineA50(windowSize, 0.5);
    MedianEngine engineA90(windowSize, 0.9);
    PercentileTrackerB trackerB(allValues);
    SortedOracle oracle;

    // Expiry follows arrival order, even if an ID was retracted.
    vector<long long> arrivalOrder;

    for (int i = 0; i < dataCount; i++) {
        // A expires automatically inside add().
        // Expire the same ID manually in B and the Oracle.
        if (static_cast<int>(arrivalOrder.size()) == windowSize) {
            long long expiredId = arrivalOrder.front();
            arrivalOrder.erase(arrivalOrder.begin());

            bool erasedB = trackerB.erase(expiredId);
            bool erasedOracle = oracle.retract(expiredId);

            if (!erasedB || !erasedOracle) {
                cout << "Could not expire ID " << expiredId
                     << " from B or the Oracle.\n";
                return 1;
            }
        }

        long long id50 = engineA50.add(data[i]);
        long long id90 = engineA90.add(data[i]);

        if (id50 != id90) {
            cout << "Structure A assigned inconsistent IDs.\n";
            return 1;
        }

        trackerB.insert(id50, data[i]);
        oracle.insert(id50, data[i]);
        arrivalOrder.push_back(id50);

        if (!compare("adding ID " + to_string(id50),
                     oracle, engineA50, engineA90, trackerB)) {
            return 1;
        }
    }

    // Retract active ID 5 from all three.
    bool removedA50 = engineA50.retract(5);
    bool removedA90 = engineA90.retract(5);
    bool removedB = trackerB.erase(5);
    bool removedOracle = oracle.retract(5);

    if (!removedA50 || !removedA90 || !removedB || !removedOracle) {
        cout << "Could not retract active ID 5 from every structure.\n";
        return 1;
    }

    if (!compare("retracting ID 5",
                 oracle, engineA50, engineA90, trackerB)) {
        return 1;
    }

    // ID 0 has expired, so retracting it should change nothing.
    removedA50 = engineA50.retract(0);
    removedA90 = engineA90.retract(0);
    removedB = trackerB.erase(0);
    removedOracle = oracle.retract(0);

    if (removedA50 || removedA90 || removedB || removedOracle) {
        cout << "An expired reading was incorrectly retracted.\n";
        return 1;
    }

    if (!compare("attempting to retract expired ID 0",
                 oracle, engineA50, engineA90, trackerB)) {
        return 1;
    }

    cout << "All differential and consistency checks passed.\n";
    return 0;
}