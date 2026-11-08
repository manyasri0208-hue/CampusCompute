CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Iinclude -Iinclude/member2
LDFLAGS =
LDLIBS = -pthread -lrt

TARGET = campuscompute

SOURCES = \
    src/main.c \
    src/jobs.c \
    src/scheduler.c \
    src/advisor.c \
    src/banker.c \
    src/logger.c \
    src/ipc.c \
    src/workers.c \
    src/limits.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
