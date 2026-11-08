#include <stdio.h>
#include "banker.h"

int bankers_safety(int process_count, int resource_count,
                   const int allocation[MAX_PROCESSES][MAX_RESOURCES],
                   const int maximum[MAX_PROCESSES][MAX_RESOURCES],
                   const int available[MAX_RESOURCES],
                   int safe_sequence[MAX_PROCESSES])
{
    int need[MAX_PROCESSES][MAX_RESOURCES];
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    int i, j, count = 0, progress;

    if (process_count <= 0 || process_count > MAX_PROCESSES ||
        resource_count <= 0 || resource_count > MAX_RESOURCES ||
        !allocation || !maximum || !available || !safe_sequence)
        return -1;

    for (j = 0; j < resource_count; ++j) {
        if (available[j] < 0)
            return -1;
        work[j] = available[j];
    }

    for (i = 0; i < process_count; ++i) {
        for (j = 0; j < resource_count; ++j) {
            if (allocation[i][j] < 0 || maximum[i][j] < allocation[i][j])
                return -1;
            need[i][j] = maximum[i][j] - allocation[i][j];
        }
    }

    while (count < process_count) {
        progress = 0;
        for (i = 0; i < process_count; ++i) {
            int can_finish = 1;
            if (finish[i])
                continue;

            for (j = 0; j < resource_count; ++j) {
                if (need[i][j] > work[j]) {
                    can_finish = 0;
                    break;
                }
            }

            if (can_finish) {
                for (j = 0; j < resource_count; ++j)
                    work[j] += allocation[i][j];
                safe_sequence[count++] = i;
                finish[i] = 1;
                progress = 1;
            }
        }
        if (!progress)
            return 0;
    }

    return 1;
}

void run_banker_demo(void)
{
    const int allocation[MAX_PROCESSES][MAX_RESOURCES] = {
        {0, 1, 0}, {2, 0, 0}, {3, 0, 2}, {2, 1, 1}, {0, 0, 2}
    };
    const int maximum[MAX_PROCESSES][MAX_RESOURCES] = {
        {7, 5, 3}, {3, 2, 2}, {9, 0, 2}, {2, 2, 2}, {4, 3, 3}
    };
    const int available[MAX_RESOURCES] = {3, 3, 2};
    int sequence[MAX_PROCESSES];
    int result, i;

    result = bankers_safety(5, 3, allocation, maximum, available, sequence);
    printf("\nBanker's Algorithm — sample state\n");
    if (result == 1) {
        printf("The system is in a SAFE state.\nSafe sequence: ");
        for (i = 0; i < 5; ++i)
            printf("P%d%s", sequence[i], (i == 4) ? "\n" : " -> ");
    } else if (result == 0) {
        printf("The system is in an UNSAFE state.\n");
    } else {
        printf("Invalid Banker input.\n");
    }
}
