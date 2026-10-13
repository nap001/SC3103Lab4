CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lpthread

all: server_challenge client_challenge

server_challenge: server_challenge.c
	$(CC) $(CFLAGS) -o server_challenge server_challenge.c $(LDFLAGS)

client_challenge: client_challenge.c
	$(CC) $(CFLAGS) -o client_challenge client_challenge.c

clean:
	rm -f server_challenge client_challenge

.PHONY: all clean
