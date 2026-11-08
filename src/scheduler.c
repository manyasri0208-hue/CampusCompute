#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

static int earlier_job(const Job *a, const Job *b)
{
    if (a->arrival_time != b->arrival_time)
        return a->arrival_time < b->arrival_time;
    return a->id < b->id;
}

static void copy_and_reset(const Job input[], int count, Job output[])
{
    int i;
    for (i = 0; i < count; ++i) {
        output[i] = input[i];
        output[i].remaining_time = input[i].burst_time;
        output[i].completion_time = 0;
        output[i].turnaround_time = 0;
        output[i].waiting_time = 0;
        output[i].response_time = -1;
        output[i].status = JOB_NEW;
    }
}

static void calculate_metrics(const Job jobs[], int count, ScheduleMetrics *m)
{
    int i;
    double wait = 0.0, tat = 0.0, response = 0.0;

    for (i = 0; i < count; ++i) {
        wait += jobs[i].waiting_time;
        tat += jobs[i].turnaround_time;
        response += jobs[i].response_time;
    }

    m->average_waiting_time = wait / count;
    m->average_turnaround_time = tat / count;
    m->average_response_time = response / count;
}

int schedule_fcfs(const Job input[], int count, Job output[],
                  ScheduleMetrics *metrics)
{
    int order[MAX_JOBS], i, j, time = 0;

    if (!input || !output || !metrics || count <= 0 || count > MAX_JOBS)
        return 0;

    copy_and_reset(input, count, output);
    for (i = 0; i < count; ++i)
        order[i] = i;

    /* Stable ordering by arrival time, then job ID. */
    for (i = 0; i < count - 1; ++i) {
        for (j = i + 1; j < count; ++j) {
            if (earlier_job(&output[order[j]], &output[order[i]])) {
                int temp = order[i];
                order[i] = order[j];
                order[j] = temp;
            }
        }
    }

    for (i = 0; i < count; ++i) {
        Job *p = &output[order[i]];
        if (time < p->arrival_time)
            time = p->arrival_time;
        p->status = JOB_RUNNING;
        p->response_time = time - p->arrival_time;
        time += p->burst_time;
        p->completion_time = time;
        p->turnaround_time = p->completion_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->burst_time;
        p->remaining_time = 0;
        p->status = JOB_COMPLETED;
    }

    calculate_metrics(output, count, metrics);
    return 1;
}

int schedule_round_robin(const Job input[], int count, int quantum, Job output[],
                         ScheduleMetrics *metrics)
{
    int queue[MAX_JOBS];
    int front = 0, rear = 0, queue_count = 0, completed = 0;
    int enqueued[MAX_JOBS] = {0};
    int time = 0, i, next;

    if (!input || !output || !metrics || count <= 0 ||
        count > MAX_JOBS || quantum <= 0)
        return 0;

    copy_and_reset(input, count, output);

    /* Begin at the earliest arrival time. */
    time = output[0].arrival_time;
    for (i = 1; i < count; ++i)
        if (output[i].arrival_time < time)
            time = output[i].arrival_time;

    while (completed < count) {
        /* Add all jobs that have arrived, in arrival-time/ID order. */
        for (;;) {
            next = -1;
            for (i = 0; i < count; ++i) {
                if (!enqueued[i] && output[i].arrival_time <= time) {
                    if (next == -1 || earlier_job(&output[i], &output[next]))
                        next = i;
                }
            }
            if (next == -1)
                break;
            if (queue_count >= MAX_JOBS)
                return 0;
            queue[rear] = next;
            rear = (rear + 1) % MAX_JOBS;
            ++queue_count;
            enqueued[next] = 1;
        }

        if (queue_count == 0) {
            /* CPU is idle: jump to the next arrival. */
            int min_arrival = -1;
            for (i = 0; i < count; ++i) {
                if (!enqueued[i] &&
                    (min_arrival == -1 ||
                     output[i].arrival_time < output[min_arrival].arrival_time))
                    min_arrival = i;
            }
            if (min_arrival == -1)
                break;
            time = output[min_arrival].arrival_time;
            continue;
        }

        {
            int index = queue[front];
            Job *p = &output[index];
            front = (front + 1) % MAX_JOBS;
            --queue_count;
            int run_for = p->remaining_time < quantum ?
                          p->remaining_time : quantum;

            if (p->response_time == -1)
                p->response_time = time - p->arrival_time;
            p->status = JOB_RUNNING;
            time += run_for;
            p->remaining_time -= run_for;

            /* Jobs arriving during this time slice join before the preempted job. */
            for (;;) {
                next = -1;
                for (i = 0; i < count; ++i) {
                    if (!enqueued[i] && output[i].arrival_time <= time) {
                        if (next == -1 || earlier_job(&output[i], &output[next]))
                            next = i;
                    }
                }
                if (next == -1)
                    break;
                if (queue_count >= MAX_JOBS)
                    return 0;
                queue[rear] = next;
                rear = (rear + 1) % MAX_JOBS;
                ++queue_count;
                enqueued[next] = 1;
            }

            if (p->remaining_time == 0) {
                p->completion_time = time;
                p->turnaround_time = time - p->arrival_time;
                p->waiting_time = p->turnaround_time - p->burst_time;
                p->status = JOB_COMPLETED;
                ++completed;
            } else {
                p->status = JOB_READY;
                if (queue_count >= MAX_JOBS)
                    return 0;
                queue[rear] = index;
                rear = (rear + 1) % MAX_JOBS;
                ++queue_count;
            }
        }
    }

    calculate_metrics(output, count, metrics);
    return 1;
}

int schedule_priority(const Job input[], int count, Job output[],
                      ScheduleMetrics *metrics)
{
    int done[MAX_JOBS] = {0};
    int completed = 0, time = 0, i, best;

    if (!input || !output || !metrics || count <= 0 || count > MAX_JOBS)
        return 0;

    copy_and_reset(input, count, output);

    while (completed < count) {
        best = -1;
        for (i = 0; i < count; ++i) {
            if (!done[i] && output[i].arrival_time <= time) {
                if (best == -1 ||
                    output[i].priority < output[best].priority ||
                    (output[i].priority == output[best].priority &&
                     earlier_job(&output[i], &output[best]))) {
                    best = i;
                }
            }
        }

        if (best == -1) {
            int next_arrival = -1;
            for (i = 0; i < count; ++i) {
                if (!done[i] &&
                    (next_arrival == -1 ||
                     output[i].arrival_time < output[next_arrival].arrival_time))
                    next_arrival = i;
            }
            if (next_arrival == -1)
                break;
            time = output[next_arrival].arrival_time;
            continue;
        }

        output[best].status = JOB_RUNNING;
        output[best].response_time = time - output[best].arrival_time;
        time += output[best].burst_time;
        output[best].completion_time = time;
        output[best].turnaround_time = time - output[best].arrival_time;
        output[best].waiting_time =
            output[best].turnaround_time - output[best].burst_time;
        output[best].remaining_time = 0;
        output[best].status = JOB_COMPLETED;
        done[best] = 1;
        ++completed;
    }

    calculate_metrics(output, count, metrics);
    return 1;
}

void display_schedule(const char *name, const Job jobs[], int count,
                      const ScheduleMetrics *metrics)
{
    int i;
    if (!name || !jobs || !metrics || count <= 0)
        return;

    printf("\n========== %s ==========", name);
    printf("\n%-6s %-7s %-7s %-7s %-7s %-7s %-7s\n",
           "ID", "AT", "BT", "CT", "TAT", "WT", "RT");
    for (i = 0; i < count; ++i) {
        printf("%-6d %-7d %-7d %-7d %-7d %-7d %-7d\n",
               jobs[i].id, jobs[i].arrival_time, jobs[i].burst_time,
               jobs[i].completion_time, jobs[i].turnaround_time,
               jobs[i].waiting_time, jobs[i].response_time);
    }
    printf("Average waiting time    : %.2f\n", metrics->average_waiting_time);
    printf("Average turnaround time : %.2f\n", metrics->average_turnaround_time);
    printf("Average response time   : %.2f\n", metrics->average_response_time);
}
