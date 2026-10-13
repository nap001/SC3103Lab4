CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lpthread

TARGET = pthread2
SRCS = pthread2.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
