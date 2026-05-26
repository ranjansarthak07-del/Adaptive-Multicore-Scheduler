# Makefile — Adaptive Multi-Algorithm Live Scheduler
#
# Targets:
#   make              → build ./scheduler_live
#   make clean        → remove build artifacts
#   make run          → build and run with default 2s poll
#   make run ARGS=5   → build and run with 5s poll interval

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11
TARGET  = scheduler_live

SRCS    = main.c proc_reader.c schedulers.c metrics.c display.c
OBJS    = $(SRCS:.c=.o)

# ── default target ────────────────────────────────────────────
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# ── compile each .c with the shared header as implicit dep ────
%.o: %.c scheduler.h
	$(CC) $(CFLAGS) -c -o $@ $<

# ── convenience targets ───────────────────────────────────────
run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: run clean
