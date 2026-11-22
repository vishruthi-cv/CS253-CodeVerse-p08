#!/usr/bin/env bash
# scripts/run_all.sh -- build, check, run the experiments and make the figures.
#
# Run from anywhere:   bash scripts/run_all.sh          (full run, N = 10^6)
#                      bash scripts/run_all.sh quick    (small test run, N = 10^5)
set -e
cd "$(dirname "$0")/.."

mkdir -p build results/raw results/figures

# On Windows (Git Bash with MinGW) the memory code needs the psapi library.
LIBS=""
case "$(uname -s)" in MINGW*|MSYS*|CYGWIN*) LIBS="-static -lpsapi" ;; esac

PY=python
command -v python >/dev/null 2>&1 || PY=python3

echo "== Building =="
g++ -std=c++17 -O2 -include string -include cstdint -include vector -include cstddef \
    -Iinclude -Isrc/structA/include -Isrc/structB/include \
    bench/bench.cpp src/structA/src/*.cpp src/structB/src/*.cpp -o build/bench $LIBS

    
echo "== Machine =="
{
  echo "date: $(date)"
  echo "compiler: $(g++ --version | head -1)"
  uname -a || true
  lscpu 2>/dev/null | head -25 || true
  echo "(on Windows, add the processor name and cache sizes by hand)"
} > results/raw/machine.txt
cat results/raw/machine.txt

echo "== Check: A and B must agree on the same stream =="
./build/bench check 1000 100000 10 50 200000 1
./build/bench check 1000 100000 10 90 200000 2
./build/bench check 50 100 30 50 100000 3

CSV=results/raw/bench.csv
rm -f "$CSV"
N=1000000     # readings per run (the project asks for at least 10^6)
REPS=5        # timed runs per point (the project asks for at least 5)
U=100000      # distinct values
P=50          # percentile: 50 = median
SEED=1
if [ "$1" = "quick" ]; then N=100000; REPS=2; fi

echo "== E1: vary the window size w (retraction rate 10%) =="
for S in A B; do
  for W in 100 1000 10000 100000; do
    ./build/bench run E1_w $S $W $U 10 $P $N $SEED $REPS "$CSV"
  done
done

echo "== E2: vary the retraction rate r (w = 10000) =="
for S in A B; do
  for R in 0 5 10 25 50; do
    ./build/bench run E2_r $S 10000 $U $R $P $N $SEED $REPS "$CSV"
  done
done

echo "== E3: vary the number of distinct values U (w = 10000, retraction rate 10%) =="
for S in A B; do
  for UU in 1000 10000 100000 1000000; do
    ./build/bench run E3_U $S 10000 $UU 10 $P $N $SEED $REPS "$CSV"
  done
done

echo "== Figures =="
$PY scripts/plot.py "$CSV" results/figures
echo "Done. Raw data: $CSV   Figures: results/figures/"