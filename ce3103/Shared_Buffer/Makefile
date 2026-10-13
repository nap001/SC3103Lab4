# Makefile for SC3103 / CE3103 - Shared Circular Buffer (Client & Server)

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lpthread

# Optional: compile with semaphore instead of mutex using `make USE_SEMAPHORE=1`
ifeq ($(USE_SEMAPHORE), 1)
	CFLAGS += -DUSE_SEMAPHORE=1
endif

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client *.o

.PHONY: all clean
