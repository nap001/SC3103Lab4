CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lpthread

TARGET = mutex1
SRCS = mutex1.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
