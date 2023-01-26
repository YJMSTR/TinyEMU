CC = gcc
CFLAGS = -Wall -o 

SRCS = $(shell find ./src/ -name '*.c')
SRCS += main.c
INCLUDES = ./include

all: 
	$(CC) $(SRCS) $(CFLAGS) main -I $(INCLUDES)

clean:
	rm -f ./main
