#ifndef JOBS_H
#define JOBS_H

#define MAX_JOBS 100

typedef enum {
    JOB_NEW = 0,
    JOB_READY,
    JOB_RUNNING,
    JOB_COMPLETED
} JobStatus;

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int status;
} Job;

int create_job(Job jobs[], int *count, int id, int arrival, int burst, int priority);
void reset_job_results(Job jobs[], int count);
void display_jobs(const Job jobs[], int count);

#endif
