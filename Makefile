CC = gcc
CFLAGS = -W -Wall -g

all: ma sv cv ag

ma: debug aux
	$(CC) $(CFLAGS) -c ma.c -o ma

sv: debug aux
	$(CC) $(CFLAGS) -c sv.c -o sv

cv: debug aux
	$(CC) $(CFLAGS) -c cv.c -o cv

ag: debug aux
	$(CC) $(CFLAGS) -c ag.c -o ag

debug:
	$(CC) $(CFLAGS) -c debug.c

aux: debug aux
	$(CC) $(CFLAGS) -c aux.c

clean:
	rm -f *.o ma sv cv ag
