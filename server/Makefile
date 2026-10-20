# Makefile para servidor MMORPG Ape Escape

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I. -I../protocol
LDFLAGS = -lpthread

SRCS = main.c server.c
OBJS = $(SRCS:.c=.o)
TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
