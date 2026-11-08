#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "workers.h"

static int report_child(pid_t child, const char *label) {
    int status = 0;
    pid_t result;
    do { result = waitpid(child, &status, 0); }
    while (result == -1 && errno == EINTR);
    if (result == -1) { perror("waitpid"); return -1; }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        printf("%s (PID %ld): exited normally with code %d.\n", label, (long)child, code);
        return code == 0 ? 0 : 1;
    }
    if (WIFSIGNALED(status)) {
        printf("%s (PID %ld): terminated by signal %d.\n", label, (long)child, WTERMSIG(status));
        return 1;
    }
    printf("%s (PID %ld): ended in an unclassified state.\n", label, (long)child);
    return 1;
}

int workers_demo(void) {
    puts("Worker monitor: starting a normal child process.");
    fflush(NULL); /* Prevent buffered menu text from being printed twice. */
    pid_t child = fork();
    if (child == -1) { perror("fork normal worker"); return -1; }
    if (child == 0) {
        printf("Normal worker (PID %ld): performing task.\n", (long)getpid());
        fflush(stdout);
        _exit(0);
    }
    if (report_child(child, "Normal worker") != 0) return -1;

    puts("Worker monitor: starting a controlled simulated-failure process.");
    fflush(NULL);
    child = fork();
    if (child == -1) { perror("fork simulated worker"); return -1; }
    if (child == 0) {
        fprintf(stderr, "Simulated worker (PID %ld): intentionally returning error code 2.\n", (long)getpid());
        _exit(2);
    }
    int result = report_child(child, "Simulated-failure worker");
    puts("Note: this is a deliberate non-zero exit status, not a real crash or system failure.");
    return result < 0 ? -1 : 0;
}
