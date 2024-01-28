CC=gcc
CFLAGS= -Wall -Wextra -Werror -latomic

all: test

test: lfh.h test.c
lfh.o: lfh.h

.PHONY:
clean:
	rm -f test *.o
