#ifndef __AUX_H__
#define __AUX_H__



typedef struct agrega{
  int quantidade;
  float preco;
} * dados;


#define TRUE  1
#define PERMISSOES 0660

#define tamArtigo 16
#define tamStocks 16
#define tamVendas 24

#define formatoArtigo "%7d %7.2f\n" //tem sete algarimos, um espaço mais sete algarimos(5 antes do ponto e 2 depois do ponto) e um \n
#define formatoStocks "%7d %7d\n"
#define formatoVendas "%7d %7d %7.2f\n"


int myread(int fildes, void *buf, int nbytes);

int myopen(const char *path, int oflag);

int mywrite(int fildes, void *buf, int nbytes);

void criaFicheiros(const char **files, int n);

int readline(int fildes, char *buf, int nbytes);

int getQuantidade(char *codigo);

int abrePipeComum();

void fechaPipeComum(int fd);

int criaPipeEspecifico();

void fechaPipeEspecifico(int fd);

int existeCodigo(int fd, int codigo, int tamformato);

#endif
