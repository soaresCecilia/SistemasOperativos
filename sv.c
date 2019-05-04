#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include "debug.h"
#include "aux.h"



/*
Função que insere uma entrada (formato: código e quantidade) no ficheiro
stocks.txt. A função devolve o número de bytes escritos.
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

	int qtos = sprintf(stocks, formatoStocks, codigoInt, quantidadeInt);

	if(qtos < 0) {
    	perror("Erro na função sprintf");
    	_exit(errno);
  	}

  	qtos = strlen(stocks);

  	DEBUG_MACRO("tamanho do formato %d\n",qtos );


    //verificar se o artigo existe

  	bytesEscritos = write(fdStocks, stocks, qtos);

  	close(fdStocks);

  	return bytesEscritos;
}



/*
Função que escreve no ecran a quantidade e o preço do artigo
cujo código passado é como parâmetro.A função retorna a quantidade
de bytes escritos no ecran.
*/
int getStock_Preco(char *codigo, int fd) {
  int bytesEscritos = 0, codigoInt, fdArt;
  int bytesLidos, cdg;
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

  int existeCod = existeCodigo(fdArt, codigoInt);

  if(!existeCod) preco = 0.0;

  bytesLidos = readline(fdArt, buffer, 1); //ver depois para ler mais bytes

  buffer[bytesLidos] = 0;

  sscanf(buffer, "%d %f", &cdg, &preco);

  sprintf(buffer, formatoArtigo, quantidade, preco);

  int qtos = strlen(buffer);

  bytesEscritos = write(fd, buffer, qtos);

  return bytesEscritos;
}


/*
Função que atualiza a quantidade dos artigos em stock.


TODO: O que fazer quando um artigo que é vendido ou atualizar o seu Stocks
mas ele não está nos artigos, qual é o preço?
Ignorava.

//TODO: quando o artigo não existia em stock e o insiro tenho de
 inserir o artigo no ficheiro artigos.txt, e se sim que nome lhe dou?


*/
int actualizaStock(char* codigo, char* quantidade){
	int fdStocks, quantidadeInt, codigoInt, nbytes, quantidadeAtual;
  int bytesEscritos, bytesLidos, sinal, quantidadeTotal;
	int flag = 0;
	char buffer[2048];
	char codigoArt[700], quantidadeArt[700];
	char c = *quantidade;

	if(c == '-'){
		 sinal = -1;
	}
	else{
		sinal = 1;
	}

	if ((fdStocks = open("stocks.txt", O_RDWR)) < 0) {
	 perror("Erro ao abrrir ficheiro stocks.txt");
	 _exit(errno);
	}

	if (lseek(fdStocks, 0, SEEK_SET) < 0){
		perror("erro no lseek");
		_exit(errno);
	}

  //caso o artigo já existe em stock
	while((bytesLidos = readline(fdStocks, buffer, 1)) > 0) {
		sscanf(buffer,"%s %s", codigoArt, quantidadeArt);

    //já leu a linha que quero mudar
		if(strcmp(codigo, codigoArt) == 0){
			buffer[0] = 0;
			codigoInt = atoi(codigo);
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
      if((nbytes = lseek(fdStocks, 0, SEEK_CUR)) < 0){
        perror("Erro ao fazer lseek");
        _exit(errno);
      }

      //linha onde está o artigo no stocks.txt
      nbytes = nbytes - tamStocks;

      //posiciona-se na linha que pretende atualizar
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

  //significa que não existia o artigo em stock
	if(flag == 0) bytesEscritos = insereStock(codigo,quantidade);

	close(fdStocks);

	return bytesEscritos;

}


/*
Função que insere uma entrada (formato código, quantidade vendida e
preço total da venda) no ficheiro vendas.txt. Caso a quantidade
passada como parametro seja superior à quantidade do produto
disponível em stock vende a quantidade do produto que tem em stock.
A função devolve o número de bytes escritos.
*/
int insereVenda(char *codigo, char *quantidade){

	int fdVendas, fdArtigos, codigoInt;
  int bytesEscritos = 0, quantidadeInt, nbytes, codNome;
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

	if((nbytes = lseek(fdArtigos, (codigoInt-1) * tamArtigo, SEEK_SET)) < 0){
		perror("Erro no lseek");
		_exit(errno);
	}

	readline(fdArtigos, buff, tamArtigo);

	sscanf(buff,"%d %f", &codNome, &preco);

  //consultar o stocks para ver quantos artigos existem em stock
  int emStock = getQuantidade(codigo);


  if(emStock == 0) return bytesEscritos;

  if(emStock > 0 && emStock < quantidadeInt) { //se tiver menos produtos em stock vende só os que tiver
    precoTotalVenda = ((float)emStock) * preco;
    sprintf(vendas, formatoVendas, codigoInt, emStock, precoTotalVenda);
  }
  else {
	precoTotalVenda = ((float)quantidadeInt) * preco;
	sprintf(vendas, formatoVendas, codigoInt, quantidadeInt, precoTotalVenda);
  }

  int qtos = strlen(vendas);

  DEBUG_MACRO("tamanho do formato %d\n",qtos );

  bytesEscritos = write(fdVendas, vendas, qtos);

  actualizaStock(codigo, quantidade);

  close(fdArtigos);

  close(fdVendas);

	return bytesEscritos;
}

/*
Função que cria o pipe para onde todos os clientes escrevem
e do qual o servidor lê.
*/
int criaPipeComum(void) {
  int fd;

  if (mkfifo("pipeComum.txt", permissoes) < 0) {
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

/*
Função que fecha o pipe para onde todos os clientes escrevem
e do qual o servidor lê.
*/
void fechaPipeComum(int fd) {
  if(close(fd) < 0) {
    perror("Erro ao fechar o pipe comum.");
    _exit(errno);
  }
}

/*
Função que divide a string com os comandos a serem executados
em várias strings, tendo como elemento separador o espaço.
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

//Função que manda executar o agregador, recebendo o numero de bytes lidos inicialmente, e que atualiza a variavel global
// do número de bytes lidos + os bytes lidos anteriormente

void mandaAgregar(int nBytesLidosAGIni){

    int nbytes;
    int status;
    int byteslidos;
    char bufferino[2048];
    int fdVendas = open("vendas.txt", O_RDONLY);

    if((nbytes = lseek(fdVendas, nBytesLidosAGIni, SEEK_SET) ) < 0){
    perror("Erro no lseek");
    _exit(errno);
  }
    //codigo do  para mandar fazer o agregador
    int pf[2];

    int fdAgFileData = open("dataagregacao.txt", O_CREAT | O_WRONLY | O_TRUNC, permissoes);
    //fazer com que o filho nasça com o output o ficheiro data




  if (pipe(pf) < 0){
    perror("Pipe PaiFilho falhou");
    _exit(errno);
  }


  switch(fork()) {
      case -1:
        perror("Fork falhou");
        _exit(errno);

      case 0:
          //filho
          dup2(fdAgFileData,1);
          close(fdAgFileData);
          //fechar descritor de escrita no pipe por parte do filho
          close(pf[1]);
          //tornar o filho capaz de ler do pipe
          dup2(pf[0],0);
          close(pf[0]);

          if((execlp("./ag","./ag",NULL))==-1){
              perror("Erro na execucao do execlp");
              _exit(errno);
          }

          _exit(errno);

      default:
          //pai
          //fechar descritor de leitura do pipe por parte do pai
          close(pf[0]);


          //escrever para o pipe
          while((byteslidos=readline(fdVendas,bufferino,1))>0){

            bufferino[byteslidos-1]='\n';
            bufferino[byteslidos]='\0';
            if(write(pf[1],bufferino,byteslidos)<0) {
                perror("Erro na escrita do file para o pipe");
            }
          }
          close(pf[1]);

    }

}

/*
Função que executa os comandos necessários para atualizar o stock e
mostrá-lo no stdout ou apenas mostrar no stdout stock e preço, consoante
sejam passados no stdin, respetivamente, o código do artigo e a quantidade
(a inserir em stock ou a vender) ou somente o código do artigo.
*/
void processaComandos(char buffer[], char *comandos, int fdCliente) {
  int conta, stock, qtos, bytesEscritos, sinal = 1;
  char codigoArt[700];
  codigoArt[0] = 0;
  char quant[700];
  quant[0] = 0;

  conta = divideComandos(comandos, codigoArt, quant);


  if(strcmp(comandos, "agregar\n") == 0) {

        mandaAgregar(0);
        return;
      }

  if(conta > 1) {
      if(quant[0] == '-') {
        sinal = -1;
        insereVenda(codigoArt, quant);
      }

      else if(sinal == 1) {
        actualizaStock(codigoArt, quant);
      }

  //rotina para imprimir no stdout
  stock = getQuantidade(codigoArt);

  sprintf(buffer, "%d\n", stock);

  qtos = strlen(buffer);

  if((bytesEscritos = write(fdCliente, buffer, qtos)) < 0) {
      perror("Erro ao escrever o pipe cliente especifico.");
      _exit(errno);
    }
  }
  else if (conta == 1) {
    getStock_Preco(codigoArt, fdCliente);
  }

}




/*
Função em que o servidor lê do pipe comum e escreve para um
pipe com nome para responder especificamente a um determinado cliente.
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

    DEBUG_MACRO("Buffer pipe Cliente %s\n", buffer);

    for(i = 0; buffer[i] != '@'; i++){
      processo[i] = buffer[i];
    }
    processo[i] = 0;
    i++;

    DEBUG_MACRO("Processo antes do .txt %s\n", processo);

    for(j = 0; buffer[i] != 0; j++, i++){
      comandos[j] = buffer[i];
    }
    comandos[j] = '\n';
    comandos[++j] = 0;

    strcat(processo, ".txt");
    DEBUG_MACRO("Processo %s Comandos %s", processo, comandos);

    //TODO: manter uma estrutura de dados para saber se o ficheiro está aberto
    if ((fdCliente = open(processo, O_RDWR)) < 0) {
      perror("Erro ao abrir o pipe cliente especifico.");
      _exit(errno);
    }

     processaComandos(buffer, comandos, fdCliente);



  }
}

//main
int main(int argc, char *argv[]) {

  const char *files[2] = {"stocks.txt", "vendas.txt"};

  criaFicheiros(files, 2);

  int fd = criaPipeComum();

  servidor(fd);

  close(fd);




  return 0;

}
