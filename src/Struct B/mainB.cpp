#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include "PercentileEngine.h"
#include "PercentileTrackerB.h"
using namespace std;


int testEngine(){
    srand(20260928);

    vector<double> pool;
    for (int i = 0; i < 2000; i++)
        pool.push_back(rand() % 5000 - 2500);

    PercentileEngine engine(pool);
    vector<double> sorted;

    for (int step = 0; step < 1000000; step++){
        bool doInsert;
        if (sorted.size() < 1000)
            doInsert = true;
        else if (sorted.size() > 3000)
            doInsert = false;
        else
            doInsert = (rand() % 2 == 0);

        if (doInsert){
            double v = pool[rand() % pool.size()];
            engine.insert(v);
            sorted.insert(upper_bound(sorted.begin(), sorted.end(), v), v);
        }
        else{
            int idx = rand() % sorted.size();
            double v = sorted[idx];
            engine.remove(v);
            sorted.erase(sorted.begin() + idx);
        }

        if (sorted.empty())
            continue;

        int n = sorted.size();
        if (engine.size() != n){
            cout << "engine: size is wrong at step " << step << endl;
            return 1;
        }

        if (engine.median() != sorted[(n + 1) / 2 - 1]){
            cout << "engine: median is wrong at step " << step << endl;
            return 1;
        }

        int p = 1 + rand() % 100;
        int k = (p * n + 99) / 100;
        if (engine.percentile(p / 100.0) != sorted[k - 1]){
            cout << "engine: percentile is wrong at step " << step << endl;
            return 1;
        }
    }
    return 0;
}

int testBasics(){
    vector<double> pool;
    pool.push_back(10);
    pool.push_back(20);
    pool.push_back(30);
    pool.push_back(40);

    PercentileTrackerB tracker(pool);

    if (tracker.getMedian() != -1.0 || tracker.getPercentile(90.0) != -1.0){
        cout << "basics: empty window must return -1.0" << endl;
        return 1;
    }

    tracker.insert(1, 10);
    tracker.insert(2, 20);
    if (tracker.getMedian() != 15.0){     
        cout << "basics: even median is wrong" << endl;
        return 1;
    }

    if (!tracker.update(2, 40) || tracker.getMedian() != 25.0){   
        cout << "basics: update is wrong" << endl;
        return 1;
    }

    if (!tracker.erase(1) || tracker.erase(1)){   
        cout << "basics: erase return value is wrong" << endl;
        return 1;
    }

    tracker.clear();
    if (!tracker.empty() || tracker.size() != 0){
        cout << "basics: clear is wrong" << endl;
        return 1;
    }
    return 0;
}

int testTracker(){
    srand(12345);

    vector<double> pool;
    for (int i = 0; i < 2000; i++)
        pool.push_back((rand() % 20000) / 10.0);

    PercentileTrackerB tracker(pool);
    vector<long long> liveIds;
    vector<double> liveValues;
    vector<double> sorted;
    long long nextId = 1;

    for (int step = 0; step < 200000; step++)
    {
        if (liveIds.size() < 500 || (liveIds.size() < 2000 && rand() % 2 == 0))
        {
            double v = pool[rand() % pool.size()];
            tracker.insert(nextId, v);
            liveIds.push_back(nextId);
            liveValues.push_back(v);
            sorted.insert(upper_bound(sorted.begin(), sorted.end(), v), v);
            nextId++;
        }
        else
        {
            int idx = rand() % liveIds.size();
            tracker.erase(liveIds[idx]);
            double v = liveValues[idx];
            sorted.erase(lower_bound(sorted.begin(), sorted.end(), v));
            liveIds.erase(liveIds.begin() + idx);
            liveValues.erase(liveValues.begin() + idx);
        }

        int n = sorted.size();
        if ((int)tracker.size() != n){
            cout << "tracker: size is wrong at step " << step << endl;
            return 1;
        }

        double expectedMedian;
        if (n % 2 == 1)
            expectedMedian = sorted[(n - 1) / 2];
        else
            expectedMedian = (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
        if (tracker.getMedian() != expectedMedian){
            cout << "tracker: median is wrong at step " << step << endl;
            return 1;
        }

        double ps[3] = {50.0, 90.0, 99.0};
        for (int j = 0; j < 3; j++)
        {
            int percent = (int)(ps[j] + 0.5);
            int k = (percent * n + 99) / 100;
            if (k < 1)
                k = 1;
            if (tracker.getPercentile(ps[j]) != sorted[k - 1])
            {
                cout << "tracker: wrong answer at step " << step << " for p = " << ps[j] << endl;
                return 1;
            }
        }
    }
    return 0;
}
int main(){
    if (testEngine() != 0)
        return 1;
    if (testBasics() != 0)
        return 1;
    if (testTracker() != 0)
        return 1;

    cout << "ALL TESTS PASSED" << endl;
    return 0;
}