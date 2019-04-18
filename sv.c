#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "debug.h"

#define tamArt 16
#define tamStocks 16
#define formatoArtigo "%7d %7.2f\n"
#define formatoStocks "%7d %7d\n"
#define formatoVendas "%7d %7d %7.2f\n"

void criaFicheiros() {
  int fdStks, fdVds;

  //criar e abrir ficheiro stocks
  fdStks = open("stocks.txt", O_CREAT | O_APPEND, 0600);

  if(fdStks < 0) {
    perror("Erro ao abrir o ficheiro stocks.txt");
    _exit(errno);
  }
  //criar e abrir ficheiro vendas
  fdVds = open("vendas.txt", O_CREAT | O_APPEND, 0600);

  if(fdVds < 0) {
    perror("Erro ao abrir o ficheiro vendas.txt");
    _exit(0);
  }

  close(fdStks);
  close(fdVds);

}

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
    return total;
}

int insereStock(char*codigo, char*quantidade){

	int fdStocks, codigoInt, quantidadeInt, bytesEscritos;
	char stocks[100];

	fdStocks = open("stocks.txt", O_WRONLY | O_APPEND);
	if(fdStocks<0){
	 perror("Erron a abrrir ficheiro stocks.txt");
	 _exit(errno);
	}

	codigoInt = atoi(codigo);
	quantidadeInt= atoi(quantidade);

	int qtos = sprintf(stocks, formatoStocks, codigoInt, quantidadeInt); //converte o codigo e quantidade numa string no formato stocks

	if(qtos < 0) {
    	perror("Erro na função sprintf");
    	_exit(errno);
  	}

  	qtos = strlen(stocks);
  	printf("tamanho do formato %d\n",qtos );

  	bytesEscritos=write(fdStocks,stocks,qtos);


  	close(fdStocks);

  	return bytesEscritos;
}

/*
TODO o que fazer quando quer vender e não existe esses artigos em stock?
O que fazer quando um artigo que é vendido ou atualizar o seu Stocks
mas ele não está nos artigos, qual é o preço?
*/
int actualizaStock(char* codigo, char* quantidade){
	int fdStocks, quantidadeInt, codigoInt, nbytes, quantidadeAtual;
  int bytesEscritos, bytesLidos, sinal, quantidadeTotal;
	int flag = 0;
	char buffer[2048];
	char codigoArt[100], quantidadeArt[100];
	char c = *quantidade;

	if(c == '-'){
		 sinal = -1;
	}
	else{
		sinal = 1;
	}


	fdStocks = open("stocks.txt", O_RDWR);

	if(fdStocks < 0){
	 perror("Erro ao abrrir ficheiro stocks.txt");
	 _exit(errno);
	}

	if (lseek(fdStocks, 0, SEEK_SET) < 0){
		perror("erro no lseek");
		_exit(errno);
	}

  DEBUG_MACRO("FD_stocks %d\n", fdStocks);

	while((bytesLidos = readline(fdStocks, buffer, 1)) > 0) {
		sscanf(buffer,"%s %s", codigoArt, quantidadeArt);

    //já leu a linha que quero mudar
		if(strcmp(codigo, codigoArt) == 0){
			buffer[0] = 0;
			codigoInt = atoi(codigo);
			quantidadeInt = abs(atoi(quantidade));
			quantidadeTotal = atoi(quantidadeArt);


      if(sinal == -1) {
        quantidadeAtual = quantidadeTotal - quantidadeInt;
        DEBUG_MACRO("Sinal %d\n", sinal);
        DEBUG_MACRO("Quantidade total %d\n", quantidadeTotal);
        DEBUG_MACRO("quantidadeInt %d\n", quantidadeInt);
        DEBUG_MACRO("quantidadeAtual %d\n", quantidadeAtual);
      }
      else {
        quantidadeAtual = quantidadeTotal + quantidadeInt;
        DEBUG_MACRO("quantidadeTotal no else %d\n", quantidadeTotal);
      }


      if((nbytes = lseek(fdStocks, 0, SEEK_CUR)) < 0){
        perror("Erro ao fazer lseek");
        _exit(errno);
      }

      DEBUG_MACRO("N_BYTES no SEEK_CUR %d\n", nbytes);

      nbytes = nbytes - tamStocks;

      DEBUG_MACRO("N_BYTES Onde quero %d\n", nbytes);

      if((nbytes = lseek(fdStocks, nbytes, SEEK_SET)) < 0) {
        perror("Erro no 2.º lseek");
        _exit(errno);
      }

			sprintf(buffer, formatoStocks, codigoInt, quantidadeAtual);

      int qtos = strlen(buffer);
      bytesEscritos = write(fdStocks, buffer, qtos);
			flag = 1;
		}
	}

	if(flag == 0) bytesEscritos = insereStock( codigo,quantidade);

	close(fdStocks);

	return bytesEscritos;

}


int insereVenda(char *codigo, char *quantidade){

	int fdVendas, fdArtigos, codigoInt, quantidadeInt, nbytes, codNome;
	float preco, precoTotalVenda;
	char buff[2048];
	char vendas[100];


	fdVendas = open("vendas.txt", O_WRONLY | O_APPEND);

	if(fdVendas < 0){
	 perror("Erron a abrrir ficheiro vendas.txt");
	 _exit(errno);
	}

	codigoInt = atoi(codigo);
	quantidadeInt = abs(atoi(quantidade));

	fdArtigos = open("artigos.txt", O_RDONLY);
	if(fdArtigos < 0){
	 perror("Erro ao abrir ficheiro artigos.txt");
	 _exit(errno);
	}


	if((nbytes = lseek(fdArtigos, (codigoInt-1) * tamArt, SEEK_SET)) < 0){
		perror("Erro no lseek");
		_exit(errno);

	}

	readline(fdArtigos, buff, tamArt);
	sscanf(buff,"%d %f", &codNome, &preco);

	precoTotalVenda = ((float)quantidadeInt) * preco;


	int qtos = sprintf(vendas, formatoVendas, codigoInt, quantidadeInt, precoTotalVenda);

	if(qtos < 0) {
    	perror("Erro na função sprintf");
    	_exit(errno);
  	}

  	qtos = strlen(vendas);

    //DEBUG_MACRO("tamanho do formato %d\n",qtos );

  	int bytesEscritos = write(fdVendas, vendas, qtos);

  	actualizaStock(codigo, quantidade);

  	close(fdArtigos);

  	close(fdVendas);

  	return bytesEscritos;
}



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


int getStock_Preco(char *codigo) {
  int bytesEscritos = 0, codigoInt, fdArt;
  int bytesfim, nbytes, bytesLidos, cdg;
  float preco;
  char buffer[1024];
  buffer[0] = 0;

  int quantidade = getQuantidade(codigo);

  codigoInt = atoi(codigo);

  fdArt = open("artigos.txt", O_RDONLY);
  if(fdArt < 0) {
    perror("Erro ao abrir o ficheiro artigos");
    _exit(errno);
  }

  if ((bytesfim = lseek(fdArt, 0,SEEK_END)) < 0) {
    perror("Erro no lseek getStock_Preco");
    _exit(errno);
  }


  if ((nbytes = lseek(fdArt, (codigoInt - 1) * tamArt,SEEK_SET)) < 0) {
    perror("Erro no lseek getStock_Preco");
    _exit(errno);
  }

  //se o artigo não existir devolve preço 0
  if (nbytes >= bytesfim) {
    preco = 0.0;
  }

   bytesLidos = readline(fdArt, buffer, 1); //ver depois para ler mais bytes

   buffer[bytesLidos] = 0;

   sscanf(buffer, "%d %f", &cdg, &preco);

   sprintf(buffer, "%7d %7.2f\n", quantidade, preco);

   int qtos = strlen(buffer);

   bytesEscritos = write(STDOUT_FILENO, buffer, qtos);

   return bytesEscritos;
}

int main(int argc, char *argv[])
{

  if(argc == 3) {
    criaFicheiros();
    if(argv[2][0]=='-'){
      insereVenda(argv[1],argv[2]);
    }
    else {
      actualizaStock(argv[1],argv[2]);
    }
  }

  else if(argc == 2) {
    getStock_Preco(argv[1]);
  }
  else printf("Escreve dois ou três argumentos\n");



	return 0;
}
