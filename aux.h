#ifndef __AUX_H__
#define __AUX_H__



#define permissoes 0600

#define tamArtigo 16
#define tamStocks 16

#define formatoArtigo "%7d %7.2f\n" //tem sete algarimos, um espaço mais sete algarimos(5 antes do ponto e 2 depois do ponto) e um \n
#define formatoStocks "%7d %7d\n"
#define formatoVendas "%7d %7d %7.2f\n"

void criaFicheiros(const char **files, int n);

int readline(int fildes, char *buf, int nbytes);

int getQuantidade(char *codigo);

int insereStock(char*codigo, char*quantidade);

int existeCodigo(int fd, int codigo);

#endif
