#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
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


/*
Função que insere uma entrada (formato: código e quantidade) no ficheiro
stocks.txt. A função devolve o número de bytes escritos.
*/
int insereStock(char*codigo, char*quantidade){

	int fdStocks, codigoInt, quantidadeInt, bytesEscritos;
	char stocks[100];

	fdStocks = open("stocks.txt", O_WRONLY | O_APPEND);
	if(fdStocks<0){
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
  	printf("tamanho do formato %d\n",qtos );

  	bytesEscritos = write(fdStocks, stocks, qtos);


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

/*
Função que insere uma entrada (formato código, quantidade vendida e
preço total da venda) no ficheiro vendas.txt. A função devolve o
número de bytes escritos.
*/
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
Função que escreve no ecran a quantidade e o preço do artigo
cujo código passado é como parâmetro.A função retorna a quantidade
de bytes escritos no ecran.
*/
int getStock_Preco(char *codigo, int fdCliente) {
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

   bytesEscritos = write(fdCliente, buffer, qtos); //escrever no pipe

   return bytesEscritos;
}


/*
Cria um pipe comum a todos os clientes
*/
int criaPipeComum(void) {
  int fd;

  if (mkfifo("pipeComum.txt", 0600) < 0) {
    if (errno != EEXIST) {
      perror("Erro ao criar o pipe comum.");
      _exit(errno);
    }
  }

  if ((fd = open("pipeComum.txt", O_RDWR)) < 0){
    perror("Erro ao abrir o ficheiro pipeComum.txt");
    _exit(errno);
  }

  return fd;
}

void fechaPipeComum(int fd) {
  if(close(fd) < 0) {
    perror("Erro ao fechar o pipe comum.");
    _exit(errno);
  }
}

/*
Função que lê do pipe e escreve para um pipe com nome para responder
especificamente a um determinado cliente.
*/
void servidor(int fd) {
  char buffer[1024];
  buffer[0] = 0;
  char processo[200];
  processo[0] = 0;
  char comandos[200];
  comandos[0] = 0;
  int byteslidos, i, j, fdCliente;


  // TODO: ler mais do que um byte de cada vez
  while((byteslidos = readline(fd, buffer, 1)) > 0){

    printf("Buffer pipe Cliente %s\n", buffer);

    //buffer[byteslidos ] = 0;

    for(i = 0; buffer[i] != '@'; i++){
      processo[i] = buffer[i];
    }
    processo[i] = 0;
    i++;

    printf("Processo antes do .txt %s\n", processo);

    for(j = 0; buffer[i] != 0; j++, i++){
      comandos[j] = buffer[i];
    }
    comandos[j] = '\n';
    comandos[++j] = 0;

    strcat(processo, ".txt");
    printf("Processo %s Comandos %s", processo, comandos);

    //TODO: manter uma estrutura de dados para saber se o ficheiro está aberto
    if ((fdCliente = open(processo, O_RDWR)) < 0) {
      perror("Erro ao abrir o pipe cliente especifico.");
      _exit(errno);
    }

    int qtos = strlen(comandos);

    getStock_Preco(comandos, fdCliente);


    /*
    if (write(fdCliente, comandos, qtos) < 0) {
      perror("Erro ao escrever no pipe do cliente.");
      _exit(errno);
    }

    */
    /*if(close(fdCliente) < 0) {
      perror("Erro ao fechar o pipe do cliente.");
      _exit(errno);
    }*/
  }

}


int main(int argc, char *argv[]) {

  int fd = criaPipeComum();

  servidor(fd);

  close(fd);

/*
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
*/


  return 0;

}
