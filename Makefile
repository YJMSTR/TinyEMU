CC = gcc
CFLAGS = -Wall -o 

SRCS = $(shell find ./src/ -name '*.c')
SRCS += main.c
INCLUDES = ./include

all: 
	$(shell mkdir build)
	$(CC) $(SRCS) $(CFLAGS) build/main -I $(INCLUDES)

clean:
	rm -rf build
