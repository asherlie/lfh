CC=gcc
CFLAGS= -Wall -Wextra -Werror -Wpedantic -Wformat -latomic -g3

all: test

test: lfh.h test.c

.PHONY:
clean:
	rm -f test *.o
