#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "jobs.h"

typedef struct {
    double average_waiting_time;
    double average_turnaround_time;
    double average_response_time;
} ScheduleMetrics;

int schedule_fcfs(const Job input[], int count, Job output[],
                  ScheduleMetrics *metrics);
int schedule_round_robin(const Job input[], int count, int quantum, Job output[],
                         ScheduleMetrics *metrics);
int schedule_priority(const Job input[], int count, Job output[],
                      ScheduleMetrics *metrics);
void display_schedule(const char *name, const Job jobs[], int count,
                      const ScheduleMetrics *metrics);

#endif
