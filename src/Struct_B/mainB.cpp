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
            tracker.retract(liveIds[idx]);
            double v = liveValues[idx];
            sorted.erase(lower_bound(sorted.begin(), sorted.end(), v));
            liveIds.erase(liveIds.begin() + idx);
            liveValues.erase(liveValues.begin() + idx);
        }

        int n = sorted.size();
        double ps[3] = {0.5, 0.9, 0.99};
        for (int j = 0; j < 3; j++)
        {

            int percent = (int)(ps[j] * 100 + 0.5);
            int k = (percent * n + 99) / 100;
            if (k < 1)
                k = 1;
            if (tracker.query(ps[j]) != sorted[k - 1])
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
    if (testTracker() != 0)
        return 1;

    cout << "ALL TESTS PASSED" << endl;
    return 0;
}