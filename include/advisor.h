#ifndef ADVISOR_H
#define ADVISOR_H

#include "scheduler.h"

typedef enum {
    OBJECTIVE_MIN_WAIT = 1,
    OBJECTIVE_MIN_RESPONSE = 2
} AdvisorObjective;

void run_scheduling_advisor(const Job jobs[], int count, int quantum,
                            AdvisorObjective objective);

#endif
