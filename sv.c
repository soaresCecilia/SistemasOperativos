#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include "debug.h"
#include "aux.h"



/*
Função que escreve no pipe de um determinado cliente.
*/
void escrevePipeCliente(int fdCliente, char *buffer, int nbytes) {
	int bytesEscritos;

	if ((bytesEscritos = mywrite(fdCliente, buffer, nbytes)) < 0 ){
		perror("Erro ao escrever no pipe do cliente na função getStock_Preco.");
	}
}

/*
Função que insere uma entrada (formato: código e quantidade) no ficheiro
stocks.
*/
void insereStock(char*codigo, char*quantidade){

	int fdArtigos, fdStocks, codigoInt, quantidadeInt, bytesEscritos = 0;
	char stocks[100];

  fdArtigos = myopen("artigos", O_RDONLY);
  if(fdArtigos < 0){
   perror("Erro a abrir ficheiro artigos na função insereStock.");
	 _exit(errno);
  }

	fdStocks = myopen("stocks", O_WRONLY | O_APPEND);
	if(fdStocks < 0){
	 perror("Erro a abrir ficheiro stocks na função insereStock.");
	 close(fdArtigos);
	 _exit(errno);
	}

	//quando querem inserir uma quantidade superior à do formato
	if(strlen(quantidade) > tamQuantidade || strlen(codigo) > tamCodigo){
		close(fdArtigos);
		close(fdStocks);
		return;
	}

	codigoInt = atoi(codigo);

	quantidadeInt = atoi(quantidade);

	sprintf(stocks, formatoStocks, codigoInt, quantidadeInt);

  int	qtos = strlen(stocks);

  DEBUG_MACRO("tamanho do formato %d    codigo do artigo %d\n",qtos, codigoInt);

  if(existeCodigo(fdArtigos,codigoInt, tamArtigo)){
			if ((bytesEscritos = mywrite(fdStocks, stocks, qtos)) < 0){
				perror("Erro ao escrever no ficheiro dos stocks na função insereStock.");
				close(fdArtigos);
				close(fdStocks);
				return;
			}
	}

  close(fdArtigos);
  close(fdStocks);
}

/*
Função que escreve no pipe especifico do cliente a quantidade
e o preço do artigo cujo código passado é como parâmetro.
*/
void getStock_Preco(char *codigo, int fdCliente) {
  int codigoInt, fdArt;
  int bytesLidos, cdg;
  float preco;
  char buffer[1024];
  buffer[0] = 0;

  int quantidade = getQuantidade(codigo);

	DEBUG_MACRO("A quantidade é : %d\n", quantidade);

  codigoInt = atoi(codigo);

	DEBUG_MACRO("O código é %d\n", codigoInt);

  fdArt = myopen("artigos", O_RDONLY);
  if(fdArt < 0) {
    perror("Erro ao abrir o ficheiro artigos na função getStock_Preco.");
		_exit(errno);
	}

  if(!existeCodigo(fdArt, codigoInt, tamArtigo)) {
    preco = 0.0;
    quantidade = 0;
  }
	else {
  	if ((bytesLidos = readline(fdArt, buffer, 1)) < 0){ //ver depois para ler mais bytes
				perror("Erro ao ler do ficheiro artigos na função getStock_Preco.");
				close(fdArt);
				return;
		}
  	buffer[bytesLidos] = 0;
  	sscanf(buffer, "%d %f", &cdg, &preco);
		DEBUG_MACRO("O buffer tem %s\n", buffer);
		DEBUG_MACRO("O codigo é %d e o preco é %f\n", cdg, preco);
	}

  sprintf(buffer, formatoArtigo, quantidade, preco);

  int qtos = strlen(buffer);

	escrevePipeCliente(fdCliente, buffer, qtos);

	DEBUG_MACRO("O que foi para o pipe do cliente foi %s\n", buffer);

	close(fdArt);
}


/*
Função que atualiza a quantidade dos artigos em stock, quer tenha
havido uma venda ou inserção em stock de um artigo.
*/
void actualizaStock(char* codigo, char* quantidade){
	int fdStocks, quantidadeInt, codigoInt, nbytes, quantidadeAtual;
  int bytesEscritos, bytesLidos, sinal, quantidadeTotal;
	char buffer[2048];
	char codigoArt[700], quantidadeArt[700];
	char c = *quantidade;

	if(c == '-') sinal = -1;

	else sinal = 1;

	//verifica se a quantidade e o código cumprem os tamanhos permitidos
	if(strlen(quantidade) > tamQuantidade || strlen(codigo) > tamCodigo){
		return;
	}

	if ((fdStocks = myopen("stocks", O_RDWR)) < 0) {
	 	perror("Erro ao abrir ficheiro stocks na função actualizaStock.");
		_exit(errno);
	}

	codigoInt = atoi(codigo);


	//artigo não existe em stock significa que também não existe nos artigos
	if (!existeCodigo(fdStocks, codigoInt, tamStocks)) {
		close(fdStocks);
		return;
	}

	else { //artigo existe em stock
		if ((bytesLidos = readline(fdStocks, buffer, 1)) < 0) {
			perror("Erro ao ler do ficheiro stocks na função actualizaStock.");
		}
		sscanf(buffer,"%s %s", codigoArt, quantidadeArt);


		quantidadeInt = abs(atoi(quantidade));
		quantidadeTotal = atoi(quantidadeArt);

		if(sinal == -1){
			if(quantidadeTotal < quantidadeInt) {
				quantidadeAtual = 0;
			}
			else quantidadeAtual = quantidadeTotal - quantidadeInt;
		}

		else quantidadeAtual = quantidadeTotal + quantidadeInt;

		//verifica onde está
		if((nbytes = lseek(fdStocks, 0, SEEK_CUR)) < 0) {
			perror("Erro ao fazer lseek na função actualizaStock.");
			close(fdStocks);
			return;
		}

		//linha onde está o artigo no stocks
		nbytes = nbytes - tamStocks;

		//posiciona-se na linha que pretende atualizar
		if((nbytes = lseek(fdStocks, nbytes, SEEK_SET)) < 0) {
			perror("Erro no 2.º lseek na função actualizaStock.");
			close(fdStocks);
			return;
		}

		sprintf(buffer, formatoStocks, codigoInt, quantidadeAtual);

		int qtos = strlen(buffer);

		if ((bytesEscritos = mywrite(fdStocks, buffer, qtos)) < 0) {
			perror("Erro ao escrever no ficheiro stocks na função actualizaStock.");
			close(fdStocks);
			return;
		}
	}

	close(fdStocks);

}

/*
Função que insere uma entrada (formato código, quantidade vendida e
preço total da venda) no ficheiro vendas. Caso a quantidade
passada como parametro seja superior à quantidade do produto
disponível em stock vende a quantidade do produto que tem em stock.
*/
void insereVenda(char *codigo, char *quantidade){
	int fdVendas, fdArtigos, codigoInt;
  int quantidadeInt, codNome;
	float preco, precoTotalVenda;
	char buff[2048];
	char vendas[100];


	fdVendas = myopen("vendas", O_WRONLY | O_APPEND);

	if(fdVendas < 0) {
	 	perror("Erro a abrir ficheiro vendas");
		_exit(errno); // TODO: ALTERAR
	}

	//se o tamanho dos comandos for superior ao permitido
	if(strlen(quantidade) > tamQuantidade || strlen(codigo) > tamCodigo){
		close(fdVendas);
		return;
	}

	codigoInt = atoi(codigo);
	quantidadeInt = abs(atoi(quantidade));

	fdArtigos = myopen("artigos", O_RDONLY);
	if(fdArtigos < 0){
	 perror("Erro ao abrir ficheiro artigos na função insereVenda.");
	 close(fdVendas);
	 _exit(errno); // TODO: ALTERAR
	}

  // TODO: VERIFICAR EM AMBOS ARTIGO E STOCK
	if (!existeCodigo(fdArtigos, codigoInt, tamArtigo)) {
		close(fdArtigos);
		close(fdVendas);
		return;
	}

	if (readline(fdArtigos, buff, 1) < 0) {
		perror("Erro a ler do ficheiro artigos na função insereVenda.");
		close(fdArtigos);
		close(fdVendas);
		return;
	}

	sscanf(buff,"%d %f", &codNome, &preco);

  //consultar o stocks para ver quantos artigos existem em stock
  int emStock = getQuantidade(codigo);

  if(emStock == 0) {
		close(fdArtigos);
		close(fdVendas);
		return;
	}

	//se tiver menos produtos em stock vende só os que tiver
  if(emStock > 0 && emStock < quantidadeInt) {
    precoTotalVenda = ((float)emStock) * preco;
    sprintf(vendas, formatoVendas, codigoInt, emStock, precoTotalVenda);
  }
  else {
		precoTotalVenda = ((float)quantidadeInt) * preco;
		sprintf(vendas, formatoVendas, codigoInt, quantidadeInt, precoTotalVenda);
  }

  int qtos = strlen(vendas);


  if(mywrite(fdVendas, vendas, qtos) < 0) {
		perror("Erro a escrever no ficheiro de vendas na função insereVenda.");
		close(fdArtigos);
		close(fdVendas);
		return;
	}

  close(fdArtigos);

  close(fdVendas);
}

/*
Função que cria o pipe para onde todos os clientes escrevem
e do qual o servidor lê. A função retorna o descritor do pipe comum.
*/
int criaPipeComum(void) {
  int fd;

  if (mkfifo("pipeComum", PERMISSOES) < 0) {
    if (errno != EEXIST) {
      perror("Erro ao criar o pipe comum.");
      _exit(errno);
    }
  }

  if ((fd = myopen("pipeComum", O_RDONLY)) < 0){
    perror("Erro ao abrir o ficheiro pipeComum");
    _exit(errno);
  }

  return fd;
}


/*
Função que divide a string com os comandos a serem executados
em várias strings, tendo como elemento separador o espaço.
A função devolve o número de string em que o parametro comandos
foi dividido.
*/
int divideComandos(char *comandos, char *codigoArt, char *quant) {
  int conta = 1, j = 0, i = 0;

   for(i = 0; comandos[i] != 0 && comandos[i] != ' '; i++){
     codigoArt[i] = comandos[i];
   }
   codigoArt[i] = 0;

   if(comandos[i] == ' ') {
     while(comandos[i] == ' ') i++;
     for(j = 0; comandos[i] != 0; i++, j++){
       quant[j] = comandos[i];
       conta++;
     }
   }
   quant[j] = 0;

   return conta;
}


/*
Função que executa os comandos necessários para atualizar o stock e
mostrá-lo no stdout ou apenas mostrar no stdout stock e preço, consoante
sejam passados no stdin, respetivamente, o código do artigo e a quantidade
(a inserir em stock ou a vender) ou somente o código do artigo.
*/
void processaComandos(char buffer[], char *comandos, int fdCliente) {
  int conta, stock, qtos, sinal = 1;
  char codigoArt[700];
  codigoArt[0] = 0;
  char quant[700];
  quant[0] = 0;

  conta = divideComandos(comandos, codigoArt, quant);

  if(conta > 1) {
      if(quant[0] == '-') {
        sinal = -1;
        insereVenda(codigoArt, quant);
				actualizaStock(codigoArt, quant);
      }

      else if(sinal == 1) {
        actualizaStock(codigoArt, quant);
      }

  		//rotina para imprimir no stdout
  		stock = getQuantidade(codigoArt);

  		sprintf(buffer, "%d\n", stock);

  		qtos = strlen(buffer);

  		escrevePipeCliente(fdCliente, buffer, qtos);
	}

	//quando só é passado o código do artigo
  else if (conta == 1) getStock_Preco(codigoArt, fdCliente);

}


void criaServidorPid() {
	char buffer[2048];
	buffer[0] = 0;

	int pidSv = getpid();

	int fd = myopen("servidorPid", O_CREAT | O_TRUNC | O_WRONLY);
	if(fd < 0) {
		perror("Erro ao criar o ficheiro servidorPid.");
		_exit(errno);
	}

	int qtos = sprintf(buffer, "%d\n", pidSv);

	if (mywrite(fd, buffer, qtos) < 0) {
		perror("Erro ao escrever no ficheiro servidorPid na função escrevePidSv.");
		_exit(errno);
	}

	close(fd);
}


/*
Função que ignora os erros ECONNRESET quando é chamada a função
readline.
A função retorna o número de bytes lidos.
*/
int myreadServidor(int fildes, void *buf, int nbytes) {
  int byteslidos;

    while(TRUE){
        byteslidos = readline(fildes, buf, nbytes);

        if (byteslidos > 0) {
          return byteslidos;
        }
        else if (errno != ECONNRESET && errno != EEXIST) {
					DEBUG_MACRO("PORRA %d %d\n", errno, byteslidos);
          return byteslidos;
        }
    }
}

/*
Função em que o servidor lê do pipe comum e escreve para um
pipe com nome para responder especificamente a um determinado cliente.
*/
void servidor(int fdComum) {
  char buffer[1024];
  buffer[0] = 0;
  char processo[200];
  processo[0] = 0;
  char comandos[1024];
  comandos[0] = 0;
  int byteslidos, i, j, fdCliente;

  // TODO: ler mais do que um byte de cada vez
  while((byteslidos = myreadServidor(fdComum, buffer, 1)) > 0) {

    	DEBUG_MACRO("Buffer pipe Cliente %s\n", buffer);

    	for(i = 0; buffer[i] != '@'; i++){
      	processo[i] = buffer[i];
    	}
    	processo[i] = 0;
    	i++;

    	for(j = 0; buffer[i] != 0; j++, i++){
      	comandos[j] = buffer[i];
    	}
    	comandos[j] = '\n';
    	comandos[++j] = 0;

			if(strlen(comandos) > PIPE_BUF) {
	        perror("Mensagem superior ao tamanho do pipe.");
					continue;
	    }

    	DEBUG_MACRO("Processo %s Comandos %s\n", processo, comandos);

    	//TODO: manter uma estrutura de dados para saber se o ficheiro está aberto
    	if ((fdCliente = myopen(processo, O_WRONLY)) < 0) {
      	perror("Erro ao abrir o pipe cliente especifico.");
      	_exit(errno);
    	}

			DEBUG_MACRO("fdCliente %d\n", fdCliente);
			processaComandos(buffer, comandos, fdCliente);
	}
}

/*
Função que inicializa a zero no ficheiro de stocks a quantidade de
todos os artigos que já foram anteriormente inseridos no ficheiro artigos.
*/
void setStocks(int signum) {
	int nbytesArt, nbytesStocks, fdArt, fdStocks;
	char buffer[100];
	buffer[0] = 0;

	(void)signum;

	fdStocks = myopen("stocks", O_RDWR);
	if(fdStocks < 0) {
	 perror("Erro a abrir ficheiro stocks na função setStocks.");
	 _exit(errno);
 }

 if((nbytesStocks = lseek(fdStocks, 0, SEEK_END)) < 0) {
		 perror("Erro ao fazer lseek no ficheiro stocks na função setStocks.");
		 close(fdStocks);
		 return;
 }

	close(fdStocks);

 fdArt = myopen("artigos", O_RDONLY);
	if (fdArt < 0) {
		perror("Erro ao abrir os artigos na função setStocks.");
		close(fdStocks);
		close(fdArt);
		_exit(errno);
	}

	if((nbytesArt = lseek(fdArt, 0, SEEK_END)) < 0) {
			perror("Erro ao fazer lseek no ficheiro artigos na função setStocks.");
			close(fdStocks);
			close(fdArt);
			return;
	}

	close(fdArt);

	int linhasArt = nbytesArt / tamArtigo;

	int linhasStocks = nbytesStocks / tamStocks;

	if(linhasStocks < linhasArt) {
		for (int i = (linhasStocks + 1); i <= linhasArt; i++) {
				sprintf(buffer, "%d", i);

				insereStock(buffer, "0");
		}
	}

}

//main
int main() {

	abrir_log("log_servidor");

  const char *files[2] = {"stocks", "vendas"};

  criaFicheiros(files, 2);

	setStocks(0);

	if (signal(SIGUSR1, setStocks) == SIG_ERR) {
		perror("Erro ao enviar o sinal.");
	}

	criaServidorPid();

  int fd = criaPipeComum();

  servidor(fd);

  fechaPipeComum(fd);

	fechar_log();

  return 0;

}
