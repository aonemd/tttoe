CC=gcc
CFLAGS=-Wall -Wextra -g

all: build run

build:
	$(CC) $(CFLAGS) -lncurses -lm -o ttt ttt.c

run:
	./ttt

clean:
	rm ttt
