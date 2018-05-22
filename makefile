.POSIX:
.SUFFIXES:
CC=gcc
CFLAGS=-g -fsanitize=address
LDFLAGS=-lasan

all: test

test: ac.o
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) 
#lib: main.o
#	ar rcs libarithlc4.a main.o

ac.o: ac.c
	$(CC) -c -o $@ $(CFLAGS) $<

debug: CFLAGS += -DDEBUG
debug: test

nolibc: CFLAGS += -DLIBCLESS -nodefaultlibs
nolibc: test

clean:
	rm *.o *.a
