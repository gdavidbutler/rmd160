CFLAGS=-I. -Os

all: rmd160 rmd320

clobber: clean
	rm -f rmd160 rmd320

clean:
	rm -f rmd160.o rmd320.o

rmd160: test/main160.c rmd160.o
	$(CC) $(CFLAGS) -o rmd160 test/main160.c rmd160.o

rmd160.o: rmd160.c rmd160.h
	$(CC) $(CFLAGS) -c rmd160.c

rmd320: test/main320.c rmd320.o
	$(CC) $(CFLAGS) -o rmd320 test/main320.c rmd320.o

rmd320.o: rmd320.c rmd320.h
	$(CC) $(CFLAGS) -c rmd320.c
