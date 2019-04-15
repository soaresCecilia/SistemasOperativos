CC = gcc
CFLAGS = -W -Wall -g

all: so
so: main.o ma.o
	$(CC) $(CFLAGS) -o so main.o ma.o

main.o: main.c ma.h
	$(CC) $(CFLAGS) -c main.c

ma.o: ma.c ma.h
	$(CC) $(CFLAGS) -c ma.c

##sv.o: sv.c sc.h
##	$(CC) $(CFLAGS) -c sv.c

##cv.o: cv.c cv.h
##	$(CC) $(CFLAGS) -c cv.c

##ag.o: ag.c ag.h
##	$(CC) $(CFLAGS) -c ag.c

clean:
	rm -f so *.o
