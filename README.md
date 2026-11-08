# CampusCompute SmartLab —

This is the Member 1 implementation:
- FR1: job creation, validation, unique IDs and job states
- FR2: FCFS, Round Robin and non-preemptive Priority scheduling
- Signature feature: measured, explainable scheduling advisor
- FR5: Banker's safety algorithm

## Build on Ubuntu / WSL

```bash
make
./campuscompute_member1
```

Or compile directly:

```bash
gcc -Wall -Wextra -std=c99 -pedantic -Iinclude \
  src/main.c src/jobs.c src/scheduler.c src/advisor.c src/banker.c \
  -o campuscompute_member1
```

## Scheduling conventions

- Lower numeric priority means higher priority.
- Equal priorities: earlier arrival time, then lower job ID.
- FCFS: earlier arrival time, then lower job ID.
- Round Robin: positive user-supplied quantum. Jobs arriving during a time slice enter the ready queue before the preempted job is requeued.
- Idle CPU time is skipped when no job has arrived.
- Turnaround = completion time - arrival time.
- Waiting = turnaround - CPU burst.
- Response = first CPU start - arrival time.
- Every algorithm works on a fresh copy of the original workload.

## Banker’s algorithm

Menu option 7 runs the standard five-process, three-resource safe example.
The function `bankers_safety()` returns:
- `1`: safe state and a safe sequence
- `0`: unsafe state
- `-1`: invalid input

## Integration note

`main.c` is a standalone Member 1 test menu. For the final two-member executable,
keep the module files and headers, and let the shared project `main.c` call these
functions. Agree on the shared `Job` structure and function prototypes with Member 2
before merging.
