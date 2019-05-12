CC = gcc
CFLAGS = -W -Wall -g

all: ma sv cv ag agf

ma: debug aux ma.c
	$(CC) $(CFLAGS) ma.c aux.o debug.o -o ma

sv: debug aux sv.c
	$(CC) $(CFLAGS) sv.c aux.o debug.o -o sv

cv: debug aux cv.c
	$(CC) $(CFLAGS) cv.c aux.o debug.o -o cv

ag: debug aux ag.c
	$(CC) $(CFLAGS) ag.c aux.o debug.o -o ag

agf: debug aux agf.c
	$(CC) $(CFLAGS) agf.c aux.o debug.o -o agf

gerar-artigos: gerar-artigos.c
	$(CC) $(CFLAGS) gerar-artigos.c -o gerar-artigos

debug:
	$(CC) $(CFLAGS) -c debug.c

aux: debug aux
	$(CC) $(CFLAGS) -c aux.c

clean:
	rm -f *.o ma sv cv ag agf gerar-artigos

clean-all-data:
	rm -f vendas artigos strings stocks log_* > /dev/null 2>&1 
