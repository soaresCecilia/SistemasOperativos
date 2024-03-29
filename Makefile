CC = gcc
CFLAGS = -W -Wall -g

all: ma sv cv ag agf gerar-artigos gerar-op-artigos gerar-op-vendas compstr

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

compstr: debug aux compstr.c
	$(CC) $(CFLAGS) compstr.c aux.o debug.o -o compstr

gerar-artigos: gerar-dados gerar-artigos.c 
	$(CC) $(CFLAGS) gerar-artigos.c gerar-dados.o -o gerar-artigos

gerar-op-artigos: gerar-dados gerar-op-artigos.c
	$(CC) $(CFLAGS) gerar-op-artigos.c gerar-dados.o -o gerar-op-artigos

gerar-op-vendas: gerar-dados gerar-op-vendas.c
	$(CC) $(CFLAGS) gerar-op-vendas.c gerar-dados.o -o gerar-op-vendas

gerar-dados: gerar-dados.c
	$(CC) $(CFLAGS) -c gerar-dados.c

debug:
	$(CC) $(CFLAGS) -c debug.c

aux: debug aux
	$(CC) $(CFLAGS) -c aux.c

clean:
	rm -f *.o ma sv cv ag agf compstr gerar-artigos gerar-op-artigos

clean-all-data:
	rm -f vendas artigos strings stocks log_* > /dev/null 2>&1 
