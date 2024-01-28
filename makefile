CC=gcc
CFLAGS= -Wall -Wextra -Werror -lpthread

all: lfh

lfh: lfh.c

.PHONY:
clean:
	rm -f lfh *.o
