#include <iostream>
#include "MedianEngine.h"

int main()
{
    MedianEngine med(5, 0.5);

    double data[] = {5, 1, 9, 3, 7, 8, 2, 6};

    for (int i = 0; i < 8; i++)
    {
        long long id = med.add(data[i]);
        std::cout << "added " << data[i] << " (id " << id << ")";
        std::cout << "  median = " << med.query() << std::endl;
    }

    med.retract(5);

    std::cout << "after retracting id 5, median = " << med.query() << std::endl;

    return 0;
}