#include "FenwickTree.h"
FenwickTree::FenwickTree(int size){
    n = size;
    tree.assign(n + 1, 0);
    topPower = 1;
    while (topPower * 2 <= n)
        topPower = topPower * 2;
}

void FenwickTree::add(int pos, int delta){
    while (pos <= n){
        tree[pos] += delta;
        pos += pos & (-pos);
    }
}

int FenwickTree::findKth(int k) const{
    int pos = 0;
    for (int step = topPower; step > 0; step = step / 2){
        if (pos + step <= n && tree[pos + step] < k){
            pos += step;
            k -= tree[pos];
        }
    }
    return pos + 1;
}