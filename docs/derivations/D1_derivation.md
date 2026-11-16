# D1 — The Two-Heap Rule (Structure A)

## 1. What we have to prove

For the median (`p = 0.5`), after every `add`, `retract`, and expiry:

1. **Balance:** `low` has the same number of live items as `high`, or one more.
2. **Order:** every live item in `low` is `<=` every live item in `high`.
3. **Median:** the top of `low` is the median.  
   For even `n`, it is the lower median.

---

## 2. Basic setup

- `low` → max heap → largest item is at the top.
- `high` → min heap → smallest item is at the top.
- `L` → number of live items in `low`.
- `H` → number of live items in `high`.
- `n = L + H` → total number of live items.
- Dead items are marked in the hash table and removed later (**lazy deletion**).

For the median:

```text
targetLow = ceil(n / 2)