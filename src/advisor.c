#include <stdio.h>
#include "advisor.h"

void run_scheduling_advisor(const Job jobs[], int count, int quantum,
                            AdvisorObjective objective)
{
    Job fcfs[MAX_JOBS], rr[MAX_JOBS], priority[MAX_JOBS];
    ScheduleMetrics mf, mr, mp;
    const char *winner;
    double vf, vr, vp;

    if (!jobs || count <= 0 || count > MAX_JOBS || quantum <= 0) {
        printf("Advisor: invalid workload or time quantum.\n");
        return;
    }

    if (!schedule_fcfs(jobs, count, fcfs, &mf) ||
        !schedule_round_robin(jobs, count, quantum, rr, &mr) ||
        !schedule_priority(jobs, count, priority, &mp)) {
        printf("Advisor: unable to run all scheduling algorithms.\n");
        return;
    }

    if (objective == OBJECTIVE_MIN_WAIT) {
        vf = mf.average_waiting_time;
        vr = mr.average_waiting_time;
        vp = mp.average_waiting_time;
        winner = "FCFS";
        if (vr < vf) { vf = vr; winner = "Round Robin"; }
        if (vp < vf) { vf = vp; winner = "Non-preemptive Priority"; }
        printf("\nAdvisor objective: minimize average waiting time.\n");
    } else if (objective == OBJECTIVE_MIN_RESPONSE) {
        vf = mf.average_response_time;
        vr = mr.average_response_time;
        vp = mp.average_response_time;
        winner = "FCFS";
        if (vr < vf) { vf = vr; winner = "Round Robin"; }
        if (vp < vf) { vf = vp; winner = "Non-preemptive Priority"; }
        printf("\nAdvisor objective: minimize average response time.\n");
    } else {
        printf("Advisor: unknown objective.\n");
        return;
    }

    printf("Measured averages for the same workload:\n");
    printf("  FCFS                    : waiting %.2f, response %.2f\n",
           mf.average_waiting_time, mf.average_response_time);
    printf("  Round Robin (q=%d)      : waiting %.2f, response %.2f\n",
           quantum, mr.average_waiting_time, mr.average_response_time);
    printf("  Non-preemptive Priority : waiting %.2f, response %.2f\n",
           mp.average_waiting_time, mp.average_response_time);
    printf("Recommendation: %s (best measured value = %.2f).\n", winner, vf);
    printf("This is a workload-specific recommendation, not a universally best algorithm.\n");
}
