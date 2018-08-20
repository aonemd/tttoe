CC=gcc
CFLAGS=-Wall -g

all: build run

build:
	$(CC) $(CFLAGS) -lncurses -o main.out main.c

run:
	./main.out

clean:
	rm main.out
