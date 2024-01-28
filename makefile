CC=gcc
CFLAGS= -Wall -Wextra -Werror -lpthread

all: test

test: lfh.h test.c
lfh.o: lfh.h

.PHONY:
clean:
	rm -f lfh *.o
