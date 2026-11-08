#include <stdio.h>
#include <stdlib.h>
#include "jobs.h"
#include "scheduler.h"
#include "advisor.h"
#include "banker.h"
#include "logger.h"
#include "ipc.h"
#include "workers.h"
#include "cc_limits.h"

static int read_int(const char *prompt, int *value)
{
    char line[128], extra;
    long parsed;

    printf("%s", prompt);
    if (!fgets(line, sizeof(line), stdin))
        return 0;

    if (sscanf(line, " %ld %c", &parsed, &extra) != 1)
        return 0;

    if (parsed < -2147483647L - 1L || parsed > 2147483647L)
        return 0;

    *value = (int)parsed;
    return 1;
}

static void add_jobs(Job jobs[], int *count)
{
    int n, i, id, at, bt, priority;

    if (!read_int("How many jobs do you want to add? ", &n) ||
        n <= 0 || n > MAX_JOBS - *count) {
        printf("Invalid job count. Maximum total is %d.\n", MAX_JOBS);
        return;
    }

    for (i = 0; i < n; ++i) {
        printf("\nJob %d details\n", i + 1);

        if (!read_int("Job ID (positive, unique): ", &id) ||
            !read_int("Arrival time (>= 0): ", &at) ||
            !read_int("CPU burst time (> 0): ", &bt) ||
            !read_int("Priority (>= 0; smaller number = higher priority): ",
                      &priority)) {
            printf("Invalid input. This job was not added.\n");
            continue;
        }

        if (!create_job(jobs, count, id, at, bt, priority))
            printf("Job rejected: check ID, values and capacity.\n");
        else
            printf("Job added successfully.\n");
    }
}

static void clean_exit(void)
{
    logger_cleanup();
    ipc_cleanup();
    puts("CampusCompute closed cleanly.");
}

int main(void)
{
    Job jobs[MAX_JOBS], result[MAX_JOBS];
    ScheduleMetrics metrics;
    int count = 0, choice, quantum;

    for (;;) {
        printf("\n===== CAMPUSCOMPUTE =====\n");
        printf("1. Add jobs\n");
        printf("2. Display jobs\n");
        printf("3. Run FCFS\n");
        printf("4. Run Round Robin\n");
        printf("5. Run non-preemptive Priority\n");
        printf("6. Compare scheduling algorithms / Smart Advisor\n");
        printf("7. Run Banker's algorithm demo\n");
        printf("8. Logger demo\n");
        printf("9. IPC demo\n");
        printf("10. Workers demo\n");
        printf("11. Limits demo\n");
        printf("12. Show logger output\n");
        printf("13. Cleanup and exit\n");

        if (!read_int("Enter choice: ", &choice)) {
            if (feof(stdin)) {
                clean_exit();
                return 0;
            }
            printf("Please enter a valid integer.\n");
            continue;
        }

        switch (choice) {
        case 1:
            add_jobs(jobs, &count);
            break;

        case 2:
            display_jobs(jobs, count);
            break;

        case 3:
            if (!schedule_fcfs(jobs, count, result, &metrics))
                printf("Add at least one valid job first.\n");
            else
                display_schedule("FCFS", result, count, &metrics);
            break;

        case 4:
            if (!read_int("Enter positive time quantum: ", &quantum) ||
                quantum <= 0) {
                printf("Time quantum must be a positive integer.\n");
            } else if (!schedule_round_robin(
                           jobs, count, quantum, result, &metrics)) {
                printf("Add valid jobs first; scheduling failed.\n");
            } else {
                display_schedule("ROUND ROBIN", result, count, &metrics);
            }
            break;

        case 5:
            if (!schedule_priority(jobs, count, result, &metrics))
                printf("Add at least one valid job first.\n");
            else
                display_schedule("NON-PREEMPTIVE PRIORITY",
                                 result, count, &metrics);
            break;

        case 6:
            if (!read_int("Enter positive Round Robin quantum: ", &quantum) ||
                quantum <= 0) {
                printf("Time quantum must be positive.\n");
                break;
            }

            printf("Choose objective:\n");
            printf("1 = Minimum average waiting time\n");
            printf("2 = Minimum average response time\n");

            if (!read_int("Objective: ", &choice) ||
                (choice != 1 && choice != 2)) {
                printf("Invalid objective.\n");
                break;
            }

            run_scheduling_advisor(
                jobs, count, quantum, (AdvisorObjective)choice);
            break;

        case 7:
            run_banker_demo();
            break;

        case 8:
            logger_demo();
            break;

        case 9:
            ipc_demo();
            break;

        case 10:
            workers_demo();
            break;

        case 11:
            limits_demo();
            break;

        case 12:
            logger_show();
            break;

        case 13:
        case 0:
            clean_exit();
            return 0;

        default:
            printf("Invalid menu choice. Enter 1-13.\n");
        }
    }
}
