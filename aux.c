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
Cria os ficheiros strings.txt e artigo.txt no diretorio onde corremos
o programa.
*/
void criaFicheiros(char *file1, char *file2) {
  int fdStr, fdArt;

  fdStr = open(file1, O_CREAT | O_APPEND, 0600);

  if(fdStr < 0) {
    perror("Erro ao abrir o ficheiro strings.txt");
    _exit(errno);
  }

  fdArt = open(file2, O_CREAT | O_APPEND, 0600);

  if(fdArt < 0) {
    perror("Erro ao abrir o ficheiro");
    _exit(0);
  }

  close(fdStr);
  close(fdArt);

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

    return (byteslidos != EOF) ? total : EOF;
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
	 perror("Erro ao abrrir ficheiro stocks.txt");
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
Função que insere uma entrada (formato: código e quantidade) no ficheiro
stocks.txt. A função devolve o número de bytes escritos.

TODO: a função que insere um produto em stock deve criar o artigo caso ele não exista
*/
int insereStock(char*codigo, char*quantidade){

	int fdStocks, codigoInt, quantidadeInt, bytesEscritos;
	char stocks[100];

	fdStocks = open("stocks.txt", O_WRONLY | O_APPEND);
	if(fdStocks < 0){
	 perror("Erro a abrrir ficheiro stocks.txt");
	 _exit(errno);
	}

	codigoInt = atoi(codigo);
	quantidadeInt = atoi(quantidade);

	int qtos = sprintf(stocks, formatoStocks, codigoInt, quantidadeInt); //converte o codigo e quantidade numa string no formato stocks

	if(qtos < 0) {
    	perror("Erro na função sprintf");
    	_exit(errno);
  	}

  	qtos = strlen(stocks);
  	DEBUG_MACRO("tamanho do formato %d\n",qtos );

  	bytesEscritos = write(fdStocks, stocks, qtos);


  	close(fdStocks);

  	return bytesEscritos;
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
if (nbytes < bytesfim)
  resultado = 1;


  return resultado;
}