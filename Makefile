# Compiler settings
CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
LDFLAGS =

# Source files
SRCS = main.c \
       src/utils.c \
       src/auth.c \
       src/inventory.c \
       src/sales.c \
       src/analytics.c

OBJS = $(SRCS:.c=.o)
TARGET = retail_shop

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
