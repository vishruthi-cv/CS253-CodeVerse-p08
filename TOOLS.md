
## Tools used

Declared use costs no marks. Each member adds their own rows below.

| Tool and version | Used by | Used for | Files affected | What we changed or verified |
|---|---|---|---|---|
| Claude (Anthropic), Claude Sonnet 5.5, Nov 2026 | Dwithi (Analyst) | Wrote the first version of the benchmark program, the plotting script and the run script; adapted them when Structure B's interface changed | bench/bench.cpp, scripts/plot.py, scripts/run_all.sh | Built and ran them on our own machine (Windows, MSYS2 g++). Fixed a crash by linking statically (-static). Checked that A and B give identical answers (bench check reports "A and B AGREE" on 3 configurations). Read through the code so I can explain it in the viva. |
| Claude (Anthropic), Claude Sonnet 5.5, Nov 2026 | Dwithi (Analyst) | Help writing the predicted cost model and derivation D6 | REPORT.md (predicted cost model), docs/derivations/D6_250001075.pdf | Checked the derivation step by step; the prediction was committed before any measurement. |
| Claude (Anthropic), Claude Sonnet 5.5, Nov 2026 | Dwithi (Analyst) | Explanations of heaps, lazy deletion, hashing, Fenwick trees, Git and the compiler, with quizzes for revision | none (study only) | None, no code or text was copied from these chats into the repository. |
| g++ (MSYS2, 16.2.0), Python 3 with matplotlib, pandas, numpy | Dwithi (Analyst) | Compiling the benchmark; making the figures | scripts/run_all.sh, scripts/plot.py | n/a |
| Gemini (Google), Nov 2026 | Neharika (Structure B owner) | Understanding Fenwick trees and binary-lifting descent, verifying edge cases in Structure B, and checking code cleanliness (std:: formatting) | include/FenwickTree.h, include/IdTable.h, include/PercentileEngine.h, include/PercentileTrackerB.h, include/SortedValueTable.h, src/structB/FenwickTree.cpp, src/structB/IdTable.cpp, src/structB/PercentileEngine.cpp, src/structB/PercentileTrackerB.cpp, src/structB/SortedValuetable.cpp, src/structB/mainB.cpp | Wrote the core implementation and verified all outputs against the sorted-vector oracle on edge cases (empty window, duplicate values, retractions). |
| Claude (Anthropic), Nov 2026 | Neharika (Structure B owner) | Checking proof steps for Derivation D2 and organizing notes for the report | docs/derivations/D2_250001073.pdf, REPORT.md (Section 5) | Rewrote the proof and report section in my own words and verified every step of the loop invariant. |
| Claude (Anthropic), Claude Sonnet 5.5, Nov 2026 | Vishruthi (Structure A owner) | Verification, bug fixing and doubts on a small part of Structure A (MedianEngine, including the checkConsistency() method) | include/MedianEngine.h, src/Struct_A/MedianEngine.cpp | I wrote Structure A myself and only used Claude for targeted fixes and to clear doubts. I kept my own code and approach, compiled and tested it myself, and checked that the output matches the teammate's oracle for p50 and p90. |

| ChatGPT (OpenAI), Codex, Nov 2026 | Anushka (Oracle owner) | Learning about differential testing and getting help developing the seeded oracle test | oracle/oracle_diff.cpp, README.md, src/structA/src/MinHeap.cpp | I provided the project requirements and interfaces. Codex helped draft the test and documentation and identify a missing `<string>` include. I checked the reported million-step run with seed `20261121`, which passed. |
<!-- Teammates: add one row per tool you used (AI assistants included), with your name,
     what it was used for, which files, and what you changed or verified. -->

## Representative prompts

Reworded from the chat. Replace with your own exact wording and dates if you kept them.

**Claude (Anthropic), Dwithi (Analyst)**

- 2026-11-12: "Write the benchmark program that feeds the same stream of readings to Structure A and Structure B, times them, measures memory, and writes a CSV, and a script that draws the graphs."
- 2026-11-14: "Structure B's interface has changed (it now takes decimal values and uses PercentileTrackerB). Update the benchmark to work with it."
- 2026-11-14: "The program crashes with a segmentation fault when I run it. Here is the output. What is wrong?"

**Gemini (Google), Neharika (Structure B owner)**

- 2026-11-15: "Explain how a Fenwick tree stores prefix frequency counts and how binary-lifting descent finds the k-th smallest element in O(log U)."
- 2026-11-16: "Check my Structure B implementation for off-by-one errors during retractions and clean up the C++17 std:: syntax."

**Claude (Anthropic), Neharika (Structure B owner)**

- 2026-11-16: "Review my proof steps for Derivation D2 (loop invariant for binary-lifting descent) and verify if any boundary condition is missing."

**Claude (Anthropic), Vishruthi (Structure A owner)**

-  "Here is my MedianEngine code and the wrong output. Can you point out the bug without changing my approach?"
-  "Is my checkConsistency() method checking the right things? I have a doubt about the heap sizes."

-  **ChatGPT (OpenAI), Codex, Anushka (Oracle owner)**

- 2026-11-21: “Help me create a seeded differential test comparing Structures A and B with a sorted-array oracle. It should check p50 and p90, check Structure A’s invariants, shrink failures, and include build and run instructions. Explain the code as we work through it.”
