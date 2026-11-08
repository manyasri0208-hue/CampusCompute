#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "logger.h"

#define LOGGER_QUEUE_CAPACITY 8
#define LOGGER_EVENT_COUNT 12
#define LOGGER_PATH "logs/campuscompute.log"

typedef struct { char message[160]; } logger_event_t;
typedef struct {
    logger_event_t queue[LOGGER_QUEUE_CAPACITY];
    int head, tail, count, producer_finished;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} logger_state_t;

static logger_state_t state;
static int state_initialized;

static void timestamp_now(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm local;
    if (localtime_r(&now, &local) == NULL ||
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &local) == 0)
        snprintf(buffer, size, "unknown-time");
}

static void *producer_thread(void *unused) {
    (void)unused;
    for (int i = 1; i <= LOGGER_EVENT_COUNT; ++i) {
        pthread_mutex_lock(&state.mutex);
        while (state.count == LOGGER_QUEUE_CAPACITY)
            pthread_cond_wait(&state.not_full, &state.mutex);
        snprintf(state.queue[state.tail].message,
                 sizeof state.queue[state.tail].message,
                 "Producer generated event %d", i);
        state.tail = (state.tail + 1) % LOGGER_QUEUE_CAPACITY;
        ++state.count;
        pthread_cond_signal(&state.not_empty);
        pthread_mutex_unlock(&state.mutex);
    }

    pthread_mutex_lock(&state.mutex);
    state.producer_finished = 1;
    pthread_cond_broadcast(&state.not_empty);
    pthread_mutex_unlock(&state.mutex);
    return NULL;
}

static void *consumer_thread(void *unused) {
    (void)unused;
    FILE *log = fopen(LOGGER_PATH, "a");
    if (log == NULL)
        fprintf(stderr, "Logger: cannot open %s: %s\n", LOGGER_PATH, strerror(errno));

    for (;;) {
        logger_event_t event;
        pthread_mutex_lock(&state.mutex);
        while (state.count == 0 && !state.producer_finished)
            pthread_cond_wait(&state.not_empty, &state.mutex);
        if (state.count == 0 && state.producer_finished) {
            pthread_mutex_unlock(&state.mutex);
            break;
        }
        event = state.queue[state.head];
        state.head = (state.head + 1) % LOGGER_QUEUE_CAPACITY;
        --state.count;
        pthread_cond_signal(&state.not_full);
        pthread_mutex_unlock(&state.mutex);

        char stamp[32];
        timestamp_now(stamp, sizeof stamp);
        printf("[%s] %s\n", stamp, event.message);
        if (log != NULL) {
            fprintf(log, "[%s] %s\n", stamp, event.message);
            fflush(log);
        }
    }
    if (log != NULL) fclose(log);
    return NULL;
}

int logger_demo(void) {
    if (mkdir("logs", 0755) == -1 && errno != EEXIST) {
        perror("Logger mkdir");
        return -1;
    }
    memset(&state, 0, sizeof state);
    int rc = pthread_mutex_init(&state.mutex, NULL);
    if (rc != 0) { fprintf(stderr, "pthread_mutex_init: %s\n", strerror(rc)); return -1; }
    rc = pthread_cond_init(&state.not_empty, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_init: %s\n", strerror(rc));
        pthread_mutex_destroy(&state.mutex);
        return -1;
    }
    rc = pthread_cond_init(&state.not_full, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_init: %s\n", strerror(rc));
        pthread_cond_destroy(&state.not_empty);
        pthread_mutex_destroy(&state.mutex);
        return -1;
    }
    state_initialized = 1;

    pthread_t producer, consumer;
    rc = pthread_create(&consumer, NULL, consumer_thread, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_create consumer: %s\n", strerror(rc));
        logger_cleanup();
        return -1;
    }
    rc = pthread_create(&producer, NULL, producer_thread, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_create producer: %s\n", strerror(rc));
        pthread_mutex_lock(&state.mutex);
        state.producer_finished = 1;
        pthread_cond_broadcast(&state.not_empty);
        pthread_mutex_unlock(&state.mutex);
        pthread_join(consumer, NULL);
        logger_cleanup();
        return -1;
    }
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    logger_cleanup();
    puts("Logger finished: events were written to logs/campuscompute.log.");
    return 0;
}

void logger_show(void) {
    FILE *log = fopen(LOGGER_PATH, "r");
    if (log == NULL) {
        puts("No event log exists yet. Run the logger first.");
        return;
    }
    puts("----- CampusCompute Event Log -----");
    char line[256];
    while (fgets(line, sizeof line, log) != NULL) fputs(line, stdout);
    fclose(log);
    puts("-----------------------------------");
}

void logger_cleanup(void) {
    if (!state_initialized) return;
    pthread_cond_destroy(&state.not_empty);
    pthread_cond_destroy(&state.not_full);
    pthread_mutex_destroy(&state.mutex);
    state_initialized = 0;
}
