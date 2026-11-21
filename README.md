# Running Median with Retractions (Project 8)

This repository evaluates two real-time percentile tracking data structures against strict SLA monitoring requirements.

## 1. Single Command Build
To compile the application from a fresh clone, run the following command from the root directory:

```bash
g++ -std=c++17 -O2 -Iinclude -Isrc/structA/include -Isrc/structB/include app/src/main.cpp src/structA/src/*.cpp src/structB/src/*.cpp -o monitor_app
