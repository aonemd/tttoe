CC=gcc
CFLAGS=-Wall -Wextra -g

all: build run

build:
	$(CC) $(CFLAGS) -lncurses -lm -o main.out main.c

run:
	./main.out

clean:
	rm main.out
