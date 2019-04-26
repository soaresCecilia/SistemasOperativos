#ifndef __AUX_H__
#define __AUX_H__

#define tamArtigo 16
#define tamStocks 16

#define formatoArtigo "%7d %7.2f\n" //tem sete algarimos, um espaço mais sete algarimos(5 antes do ponto e 2 depois do ponto) e um \n
#define formatoStocks "%7d %7d\n"
#define formatoVendas "%7d %7d %7.2f\n"

void criaFicheiros(char *file1, char *file2);

int readline(int fildes, char *buf, int nbytes);

int getQuantidade(char *codigo);

int insereStock(char*codigo, char*quantidade);

#endif
