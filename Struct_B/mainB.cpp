#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include "PercentileEngine.h"
using namespace std;

int main(){
    srand(20260928);

    vector<long long> pool;
    for (int i = 0; i < 2000; i++)
        pool.push_back(rand() % 5000 - 2500);

    PercentileEngine engine(pool);
    vector<long long> sorted;

    for (int step = 0; step < 1000000; step++)
    {
        bool doInsert;
        if (sorted.size() < 1000)
            doInsert = true;
        else if (sorted.size() > 3000)
            doInsert = false;
        else
            doInsert = (rand() % 2 == 0);

        if (doInsert){
            long long v = pool[rand() % pool.size()];
            engine.insert(v);
            sorted.insert(upper_bound(sorted.begin(), sorted.end(), v), v);
        }
        else{
            int idx = rand() % sorted.size();
            long long v = sorted[idx];
            engine.remove(v);
            sorted.erase(sorted.begin() + idx);
        }

        if (sorted.empty())
            continue;

        int n = sorted.size();
        if (engine.size() != n)
        {
            cout << "size is wrong at step " << step <<endl;
            return 1;
        }

        if (engine.median() != sorted[(n + 1) / 2 - 1])
        {
            cout << "median is wrong at step " << step <<endl;
            return 1;
        }

        int p = 1 + rand() % 100;
        int k = (p * n + 99) / 100;
        if (engine.percentile(p) != sorted[k - 1])
        {
            cout << "percentile is wrong at step " << step <<endl;
            return 1;
        }
    }

    cout << "ALL TESTS PASSED" <<endl;
    return 0;
}