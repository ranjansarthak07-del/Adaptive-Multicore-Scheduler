#  Adaptive Multi-Core Load Balanced Scheduler

A high-performance CPU scheduling system that dynamically selects the optimal scheduling algorithm (SJF, RR, FCFS) based on real-time process behavior, designed for multi-core environments.

---

##  Overview

Modern operating systems face highly dynamic workloads where no single scheduling algorithm performs optimally in all scenarios.

This project introduces an **adaptive hybrid scheduling framework** that:

- Monitors live system processes using the Linux `/proc` filesystem
- Estimates CPU burst time in real-time
- Dynamically selects the most efficient scheduling strategy per process
- Distributes workload across multiple CPU cores

---

##  Key Features

-  **Adaptive Scheduling**
  - Automatically switches between:
    - Shortest Job First (SJF)
    - Round Robin (RR)
    - First Come First Serve (FCFS)

-  **Multi-Core Load Balancing**
  - Efficient distribution across 2 CPU cores
  - Reduces idle time and improves throughput

-  **Real-Time Monitoring**
  - Uses `/proc` to read live system processes
  - No synthetic data — works on real workloads

-  **Performance Metrics**
  - Waiting Time
  - Turnaround Time
  - Throughput
  - CPU Utilization
  - Context Switching

-  **Live Dashboard Output**
  - Terminal-based visualization of scheduling decisions

---

##  Scheduling Strategy

| Burst Size | Algorithm Used |
|-----------|---------------|
| Short     | SJF           |
| Medium    | Round Robin   |
| Long      | FCFS          |

This ensures:
-  Fast execution for short tasks  
-  Fairness for medium tasks  
-  Stability for long-running processes  

---

##  System Architecture
Processes (/proc)
↓
Burst Estimation
↓
Ready Queue
↓
Adaptive Decision Engine
↓
Multi-Core Scheduler
↓
Execution + Metrics


---

##  Workflow

1. Scan `/proc` for active processes  
2. Estimate CPU burst time  
3. Add processes to ready queue  
4. Select scheduling algorithm dynamically  
5. Assign processes to available cores  
6. Execute and update metrics  
7. Repeat continuously  

---

##  Results

The adaptive scheduler demonstrates:

-  Reduced average waiting time  
-  Improved turnaround time  
-  Higher throughput  
-  Balanced CPU utilization  
-  Controlled context switching  

Compared to:
- FCFS  
- SJF  
- Round Robin  

---

##  How to Run

```bash
make
./scheduler_live
