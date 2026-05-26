# Adaptive Multi-Core Scheduler

This project implements an adaptive CPU scheduling system that dynamically selects between FCFS, SJF, and Round Robin based on process behavior.

## Features
- Multi-core scheduling (2 cores)
- Dynamic algorithm selection
- Real-time process monitoring using /proc
- Performance metrics (waiting time, turnaround, throughput)

## How to Run
make
./scheduler_live

## Algorithms Used
- FCFS
- SJF
- Round Robin
- Adaptive Hybrid Scheduler