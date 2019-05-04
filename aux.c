#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "debug.h"
#include "aux.h"

/*
Cria os ficheiros no diretorio onde corremos o programa.
*/
void criaFicheiros(const char *file[], int n) {
  int fd;

  for(int i = 0; i < n; i++) {
    fd = open(file[i], O_CREAT | O_APPEND, permissoes);

    if(fd < 0) {
    perror("Erro ao abrir os ficheiros.");
    _exit(errno);
    }

    close(fd);
  }

}

/*
Lê uma linha do ficheiro, nbytes de cada vez, devolvendo o número
total de bytes lidos.

TODO: se total for maior do que o buffer tem de fazer realloc do buffer. Tb ter
em atenção que o buffer pode não aguentar uma linha completa.

*/
int readline(int fildes, char *buf, int nbytes) {
    int byteslidos, total = 0;

    while((byteslidos = read(fildes, buf, nbytes)) > 0){
      total += byteslidos; //vai contando o total de bytes que lê
      if(*buf == '\n') {
        *buf = 0;
        break; // se o char que está naquela posição do buffer for newline
      }
      buf = buf + byteslidos; //aumenta a posição no buffer onde o caracter vai ser armazenado
    }

    return (byteslidos > 0) ? total : byteslidos;
}

/*
Função que devolve a quantidade do
artigo, com o código passado como parâmetro, com base
na informação obtida no ficheiro stocks.txt.
*/
int getQuantidade(char *codigo) {
  int bytesLidos, quantidade = 0, codigoArt = 0;
  char buffer[1024];
  int fdStocks = open("stocks.txt", O_RDWR);

  int codigoInt = atoi(codigo);

	if(fdStocks < 0){
	 perror("Erro ao abrir ficheiro stocks.txt");
	 _exit(errno);
	}

	if (lseek(fdStocks, 0, SEEK_SET) < 0){
		perror("erro no lseek");
		_exit(errno);
	}

  while((bytesLidos = readline(fdStocks, buffer, 1)) > 0) {
    sscanf(buffer,"%d %d", &codigoArt, &quantidade);

    //já leu a linha que quero mudar
    if(codigoInt == codigoArt){
      return quantidade;
    }
    else quantidade = 0; //o código não existe nos stocks
  }

  return quantidade;
}

/*
Função que verifica se o código introduzido existe no ficheiro.
*/
int existeCodigo(int fd, int codigoInt) {
  int resultado = 0, nbytes, bytesfim;

if ((bytesfim = lseek(fd, 0, SEEK_END)) < 0) {
  perror("Erro no lseek a partir do fim.");
  _exit(errno);
}

if ((nbytes = lseek(fd, (codigoInt - 1) * tamArtigo, SEEK_SET)) < 0) {
  perror("Erro no lseek da posicao do artigo");
  _exit(errno);
}

//se o artigo não existir devolve preço 0 o nbytes tem de ser menor que bytesfim não pode ser igual
if (nbytes < bytesfim) resultado = 1;

  return resultado;
}
