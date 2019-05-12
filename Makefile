CC = gcc
CFLAGS = -W -Wall -g

all: ma sv cv ag agf

ma: debug aux
	$(CC) $(CFLAGS) ma.c aux.o debug.o -o ma

sv: debug aux
	$(CC) $(CFLAGS) sv.c aux.o debug.o -o sv

cv: debug aux
	$(CC) $(CFLAGS) cv.c aux.o debug.o -o cv

ag: debug aux
	$(CC) $(CFLAGS) ag.c aux.o debug.o -o ag

agf: debug aux
	$(CC) $(CFLAGS) agf.c aux.o debug.o -o agf

debug:
	$(CC) $(CFLAGS) -c debug.c

aux: debug aux
	$(CC) $(CFLAGS) -c aux.c

clean:
	rm -f *.o ma sv cv ag agf

clean-all-data:
	rm -f vendas artigos strings stocks log_* > /dev/null 2>&1 
