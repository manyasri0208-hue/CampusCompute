#ifndef BANKER_H
#define BANKER_H

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

/*
 * Returns 1 when the state is safe, 0 when unsafe, and -1 for invalid input.
 * safe_sequence receives process indices in safe execution order.
 */
int bankers_safety(int process_count, int resource_count,
                   const int allocation[MAX_PROCESSES][MAX_RESOURCES],
                   const int maximum[MAX_PROCESSES][MAX_RESOURCES],
                   const int available[MAX_RESOURCES],
                   int safe_sequence[MAX_PROCESSES]);

void run_banker_demo(void);

#endif
