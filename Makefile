CC = gcc
CFLAGS = -g -Wall -Wextra -Wpedantic
INCLUDES = -I.
SRCS = memdoor.c process.c network.c utils.c
OBJS = $(SRCS:.c=.o)
TARGET = memdoor

.PHONY: all clean static

all: $(TARGET)

static: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -static -o $(TARGET) $^

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
