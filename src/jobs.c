#include <stdio.h>
#include "jobs.h"

int create_job(Job jobs[], int *count, int id, int arrival, int burst, int priority)
{
    int i;

    if (jobs == NULL || count == NULL || *count < 0 || *count >= MAX_JOBS)
        return 0;
    if (id <= 0 || arrival < 0 || burst <= 0 || priority < 0)
        return 0;

    for (i = 0; i < *count; ++i) {
        if (jobs[i].id == id)
            return 0;
    }

    jobs[*count].id = id;
    jobs[*count].arrival_time = arrival;
    jobs[*count].burst_time = burst;
    jobs[*count].priority = priority;
    jobs[*count].remaining_time = burst;
    jobs[*count].completion_time = 0;
    jobs[*count].turnaround_time = 0;
    jobs[*count].waiting_time = 0;
    jobs[*count].response_time = -1;
    jobs[*count].status = JOB_NEW;
    ++(*count);
    return 1;
}

void reset_job_results(Job jobs[], int count)
{
    int i;
    if (jobs == NULL || count < 0)
        return;

    for (i = 0; i < count; ++i) {
        jobs[i].remaining_time = jobs[i].burst_time;
        jobs[i].completion_time = 0;
        jobs[i].turnaround_time = 0;
        jobs[i].waiting_time = 0;
        jobs[i].response_time = -1;
        jobs[i].status = JOB_NEW;
    }
}

void display_jobs(const Job jobs[], int count)
{
    int i;

    if (jobs == NULL || count <= 0) {
        printf("No jobs available.\n");
        return;
    }

    printf("\n%-8s %-10s %-10s %-10s\n",
           "Job ID", "Arrival", "Burst", "Priority");
    for (i = 0; i < count; ++i) {
        printf("%-8d %-10d %-10d %-10d\n",
               jobs[i].id, jobs[i].arrival_time,
               jobs[i].burst_time, jobs[i].priority);
    }
}
