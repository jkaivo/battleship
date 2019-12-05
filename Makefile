.POSIX:

all: bs

OBJS=bs.o
LIBS=-lcurses

CC=c99
CFLAGS=-Wall -Wextra -Wpedantic

bs: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f bs *.o
