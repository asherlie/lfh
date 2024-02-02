CC=gcc
CFLAGS= -Wall -Wextra -Werror -Wpedantic -Wformat -latomic -g3

all: test

install: lfh.h
	install -m 644 lfh.h /usr/local/include/

test: lfh.h test.c

.PHONY:
clean:
	rm -f test *.o
