#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include "cc_limits.h"

int limits_demo(void) {
    struct rlimit original;
    if (getrlimit(RLIMIT_NOFILE, &original) == -1) {
        perror("getrlimit"); return -1;
    }
    printf("Parent process current open-file limit: soft=%llu, hard=%llu.\n",
           (unsigned long long)original.rlim_cur,
           (unsigned long long)original.rlim_max);

    fflush(NULL); /* Prevent buffered menu text from being printed twice. */
    pid_t child = fork();
    if (child == -1) { perror("fork resource-limit child"); return -1; }
    if (child == 0) {
        struct rlimit restricted;
        restricted.rlim_cur = 16;
        restricted.rlim_max = 16;
        if (setrlimit(RLIMIT_NOFILE, &restricted) == -1) {
            perror("child setrlimit"); _exit(1);
        }
        puts("Child: RLIMIT_NOFILE set to 16. Opening /dev/null repeatedly...");
        fflush(stdout);
        int opened = 0;
        for (;;) {
            int fd = open("/dev/null", O_RDONLY);
            if (fd == -1) {
                printf("Child: open() stopped after %d successful opens: %s\n", opened, strerror(errno));
                break;
            }
            ++opened; /* Keep each descriptor open to reach the process limit. */
        }
        /* The child exits immediately; the kernel closes its descriptors. */
        fflush(stdout);
        _exit(0);
    }

    int status = 0;
    pid_t result;
    do { result = waitpid(child, &status, 0); }
    while (result == -1 && errno == EINTR);
    if (result == -1) { perror("waitpid resource-limit child"); return -1; }
    if (WIFEXITED(status))
        printf("Parent: child exit code %d. Parent's resource limit remains unchanged.\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("Parent: child terminated by signal %d.\n", WTERMSIG(status));
    return 0;
}
