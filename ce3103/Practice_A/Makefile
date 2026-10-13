CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lpthread

TARGET = pthread1
SRCS = pthread1.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
