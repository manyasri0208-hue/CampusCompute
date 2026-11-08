#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ipc.h"

#define CC_SHM_NAME "/campuscompute_member2_shm"
#define CC_SEM_NAME "/campuscompute_member2_sem"
#define CC_SHM_SIZE 256

static int owns_shared_memory;
static int owns_semaphore;

static int wait_for_child(pid_t child, int *status) {
    pid_t result;
    do { result = waitpid(child, status, 0); }
    while (result == -1 && errno == EINTR);
    if (result == -1) { perror("waitpid (IPC child)"); return -1; }
    return 0;
}

int ipc_demo(void) {
    int fd = shm_open(CC_SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1) {
        if (errno == EEXIST)
            fprintf(stderr, "IPC: %s already exists; refusing to remove an object not created by this run.\n", CC_SHM_NAME);
        else perror("shm_open");
        return -1;
    }
    owns_shared_memory = 1;

    if (ftruncate(fd, CC_SHM_SIZE) == -1) {
        perror("ftruncate"); close(fd); ipc_cleanup(); return -1;
    }
    char *shared = mmap(NULL, CC_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (shared == MAP_FAILED) {
        perror("mmap"); ipc_cleanup(); return -1;
    }
    shared[0] = '\0';

    sem_t *gate = sem_open(CC_SEM_NAME, O_CREAT | O_EXCL, 0600, 0);
    if (gate == SEM_FAILED) {
        if (errno == EEXIST) fprintf(stderr, "IPC: semaphore already exists; leaving it untouched.\n");
        else perror("sem_open");
        munmap(shared, CC_SHM_SIZE); ipc_cleanup(); return -1;
    }
    owns_semaphore = 1;

    fflush(NULL); /* Do not duplicate buffered menu/output across fork(). */
    pid_t child = fork();
    if (child == -1) {
        perror("fork (IPC consumer)");
        sem_close(gate); munmap(shared, CC_SHM_SIZE); ipc_cleanup(); return -1;
    }
    if (child == 0) {
        if (sem_wait(gate) == -1) { perror("child sem_wait"); _exit(2); }
        printf("Child consumer (PID %ld): read shared message: %s\n", (long)getpid(), shared);
        fflush(stdout);
        sem_close(gate);
        munmap(shared, CC_SHM_SIZE);
        _exit(0);
    }

    snprintf(shared, CC_SHM_SIZE, "Hello from the CampusCompute parent process.");
    printf("Parent producer (PID %ld): wrote message into shared memory.\n", (long)getpid());
    if (sem_post(gate) == -1) {
        perror("parent sem_post");
        /* Wake the child if possible so it cannot remain blocked forever. */
        kill(child, SIGTERM);
        int ignored; wait_for_child(child, &ignored);
        sem_close(gate); munmap(shared, CC_SHM_SIZE); ipc_cleanup(); return -1;
    }
    puts("Parent: semaphore posted; child is allowed to read.");

    int status = 0;
    int wait_result = wait_for_child(child, &status);
    sem_close(gate);
    munmap(shared, CC_SHM_SIZE);
    if (wait_result != 0) { ipc_cleanup(); return -1; }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "IPC child did not complete successfully.\n");
        ipc_cleanup(); return -1;
    }
    puts("IPC exchange completed successfully.");
    ipc_cleanup();
    return 0;
}

void ipc_cleanup(void) {
    if (owns_semaphore) {
        if (sem_unlink(CC_SEM_NAME) == -1 && errno != ENOENT) perror("sem_unlink");
        owns_semaphore = 0;
    }
    if (owns_shared_memory) {
        if (shm_unlink(CC_SHM_NAME) == -1 && errno != ENOENT) perror("shm_unlink");
        owns_shared_memory = 0;
    }
}
