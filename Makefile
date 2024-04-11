CFLAGS=-I. -Os

all: rmd160

clobber: clean
	rm -f rmd160

clean:
	rm -f rmd160.o

rmd160: test/main.c rmd160.o
	$(CC) $(CFLAGS) -o rmd160 test/main.c rmd160.o

rmd160.o: rmd160.c rmd160.h
	$(CC) $(CFLAGS) -c rmd160.c
