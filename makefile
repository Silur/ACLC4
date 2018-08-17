.POSIX:
.SUFFIXES:
CC=gcc
CFLAGS=-g -fsanitize=address -fPIC
LDFLAGS=-lasan -shared

all: libaclc4.so

libaclc4.so: ac.o lc4.o
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

ac.o: ac.c
	$(CC) -c -o $@ $(CFLAGS) $<

lc4.o: lc4.c
	$(CC) -c -o $@ $(CFLAGS) $<

test: libaclc4.so test.c
	$(CC) -o $@ $(CFLAGS) test.c -laclc4 -lasan

debug: CFLAGS += -DDEBUG

nolibc: CFLAGS += -DLIBCLESS -nodefaultlibs
nolibc: libaclc4

clean:
	rm *.o *.so test
