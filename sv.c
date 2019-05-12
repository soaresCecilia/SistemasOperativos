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
#include <time.h> 
#include "debug.h"
#include "aux.h"

static int posicaoLivre = 0;

visitado inicializaVisitado(int cod, float precoArt) {

  visitado novo = malloc(sizeof(struct artigoVisitado));

  novo->codArtigo = cod;
  novo->precoArtigo = precoArt;
  novo->vezesVisitado = 0;

  return novo;

}

void inicializaArray(visitado artigosVisitados[]) {

  int i;

  for(i = 0; i < tamCache; i++) {

    artigosVisitados[i] = NULL;
  
  }

}

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

void atualizaMaisVisitados(int codigoArtigo, int flag, float preco, visitado artigosVisitados[]) {

  int i;
  int encontrado = 0;

  if(flag == 1) {

    for(i = 0; i < tamCache && encontrado == 0; i++) {

      if(codigoArtigo == artigosVisitados[i]->codArtigo) {
        artigosVisitados[i]->vezesVisitado += 1;
        printf("Vezes visitado = %d\n", artigosVisitados[i]->vezesVisitado);
        encontrado = 1;
      }

    }

  }

  else {

    if(posicaoLivre == tamCache) {

      visitado novoArtigo = malloc(sizeof(struct artigoVisitado));//inicializaVisitado();

      (*novoArtigo).precoArtigo = preco;
      (*novoArtigo).vezesVisitado = 1;
      (*novoArtigo).codArtigo = codigoArtigo;

      for(i = 1; i < tamCache; i++)
        artigosVisitados[i-1] = artigosVisitados[i];

      artigosVisitados[tamCache-1] = novoArtigo;

    }

    else {

      visitado novoArtigo = malloc(sizeof(struct artigoVisitado));//inicializaVisitado();

      (*novoArtigo).precoArtigo = preco;
      (*novoArtigo).vezesVisitado = 1;
      (*novoArtigo).codArtigo = codigoArtigo;

      artigosVisitados[posicaoLivre] = novoArtigo;
      posicaoLivre += 1;

    }

  }

}


int existeProdutoMaisVisitado(int codigoArtigo, visitado artigosVisitados[]) {

  int i, final = -1;
  int encontrado = 0;

  printf("Posicao livre %d\n", posicaoLivre);

  if(posicaoLivre == 0)
    return -1;

  for(i = 0; i < tamCache && encontrado == 0 && artigosVisitados[i] != NULL; i++) {

    //printf("Codigo do artigo é: %d\n", artigosVisitados[i]->codArtigo);

    if(codigoArtigo == artigosVisitados[i]->codArtigo) {

      final = i;

      encontrado = 1;

    }

  }

  return final;

}

/*
Função que escreve no pipe especifico do cliente a quantidade
e o preço do artigo cujo código passado é como parâmetro.
*/
void getStock_Preco(char *codigo, int fdCliente, visitado artigosVisitados[]) {
  int codigoInt, fdArt;
  int bytesLidos, cdg;
  float preco;
  char buffer[1024];
  buffer[0] = 0;
  int existeArray = -1;

  int quantidade = getQuantidade(codigo);

	DEBUG_MACRO("A quantidade é : %d\n", quantidade);

  codigoInt = atoi(codigo);

	DEBUG_MACRO("O código é %d\n", codigoInt);

  fdArt = myopen("artigos", O_RDONLY);
  if(fdArt < 0) {
    perror("Erro ao abrir o ficheiro artigos na função getStock_Preco.");
		_exit(errno);
	}

  if(CACHE)
    existeArray = existeProdutoMaisVisitado(codigoInt, artigosVisitados);
  printf("EXISTE ARRAY: %d\n", existeArray);

  if(existeArray >= 0) {

    preco = (*artigosVisitados[existeArray]).precoArtigo;

    sprintf(buffer, formatoArtigo, quantidade, preco);

    int qtos = strlen(buffer);

    atualizaMaisVisitados(codigoInt, 1, preco, artigosVisitados);

    escrevePipeCliente(fdCliente, buffer, qtos);

    DEBUG_MACRO("O que foi para o pipe do cliente foi %s\n", buffer);

  }
  else {

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

  atualizaMaisVisitados(codigoInt, 0, preco, artigosVisitados);

  escrevePipeCliente(fdCliente, buffer, qtos);

  DEBUG_MACRO("O que foi para o pipe do cliente foi %s\n", buffer);

  }

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


// codigo para criar nome ficheiro com o resultado da agregacao
void nomeFichAgregar(char *dataHora) {


    time_t rawtime = time(NULL);
    
    if (rawtime == -1) {
        perror("The time() function failed");
    }
  
    struct tm *ptm = localtime(&rawtime);
    
    if (ptm == NULL) {
       perror("The localtime() function failed");    
    }
    
    sprintf(dataHora,"%d-%02d-%02dT%02d:%02d:%02d",ptm->tm_year+1900,ptm->tm_mon,ptm->tm_mday, ptm->tm_hour, 
           ptm->tm_min, ptm->tm_sec);
}

int fsize(char * ficheiro){
    FILE * fp = fopen(ficheiro, "r");

    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //voltar ao inicio do ficheiro
    fclose(fp);
    return sz;
}

/*
Função que manda executar o agregador, recebendo o numero de bytes lidos
inicialmente, e que atualiza a variavel global
do número de bytes lidos + os bytes lidos anteriormente
*/
int mandaAgregar(int nBytesLidosAGIni, int ii){
    int nbytes;
    int posicao = 0;
    char dataHora[100];
    dataHora[0]= 0;
    int byteslidos;
    char bufferino[2048];
    char iiBuffer[20];
    sprintf(iiBuffer, "%d", ii);
    strcat(iiBuffer, ".txt");
    int tamFicheiro = fsize("vendas");
    int numeroBarraN = 0;

    if(ii == 1)
      posicao = ((tamFicheiro / tamVendas) / 2);

    int fdVendas = myopen("vendas", O_RDONLY);
    if (fdVendas < 0) {
      perror("Erro ao abrir o ficheiro vendas na função mandaAgregar.");
      _exit(errno);
    }

    if(nBytesLidosAGIni >= tamFicheiro) {
      perror("Erro no lseek");
      _exit(errno);
    }

    if(nBytesLidosAGIni == 0) {

      if((nbytes = lseek(fdVendas, nBytesLidosAGIni + (((tamFicheiro - nBytesLidosAGIni)/tamVendas)/2)*(ii*tamVendas), SEEK_SET) ) < 0){
        perror("Erro no lseek");
        _exit(errno);
      }

    }
    else {

      if((nbytes = lseek(fdVendas, nBytesLidosAGIni, SEEK_SET) ) < 0){
        perror("Erro no lseek");
        _exit(errno);
      }

    }

    

    printf("NUMERO DE BYTES ONDE ESTOU: %d\n", nbytes);

    
    //codigo do  para mandar fazer o agregador
    int pf[2];

    nomeFichAgregar(dataHora);

    int fdAgFileData = myopen(iiBuffer,O_TRUNC);
    close(fdAgFileData);
    fdAgFileData = myopen(iiBuffer, O_CREAT | O_WRONLY);
    if (fdAgFileData < 0) {
      perror("Erro no lseek na função mandaAgregar.");
      close(fdVendas);
  }
    
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
          if(ii == 1) {

            while((byteslidos=readline(fdVendas,bufferino,1))>0){

            bufferino[byteslidos-1]='\n';
            bufferino[byteslidos]='\0';
            if(mywrite(pf[1],bufferino,byteslidos)<0) {
                perror("Erro na escrita do ficheiro vendas para o pipe.");
            }
            posicao++;
          }
        } 

          else {

            while((byteslidos=readline(fdVendas,bufferino,1))>0 && numeroBarraN < ((tamFicheiro-nBytesLidosAGIni)/tamVendas)/2){

            bufferino[byteslidos-1]='\n';
            numeroBarraN++;
            bufferino[byteslidos]='\0';
            if(mywrite(pf[1],bufferino,byteslidos)<0) {
                perror("Erro na escrita do ficheiro vendas para o pipe.");
            }
            posicao++;
          }

          }          
          
          close(pf[1]);
    }

return posicao;

}

int mandaAgregarGeral(int nBytesLidosAGIni){
    int nbytes;
    int posicao = 0;
    char dataHora[100];
    dataHora[0]= 0;
    int byteslidos;
    char bufferino[2048];
    char bufferino2[2048];
    char iiBuffer[20];
    int filho;
    int numeroBarraN = 0;

    int fdVendas = myopen("vendas", O_RDONLY);
    if (fdVendas < 0) {
      perror("Erro ao abrir o ficheiro vendas na função mandaAgregar.");
      _exit(errno);
    }

    /*if((nbytes = lseek(fdVendas, nBytesLidosAGIni + ((tamFicheiro/tamVendas)/4)*(tamVendas*ii), SEEK_SET) ) < 0){
        perror("Erro no lseek");
        _exit(errno);
    }*/

    
    //codigo do  para mandar fazer o agregador
    int pf[2];

    nomeFichAgregar(dataHora);

    int fdAgFileData = myopen(dataHora, O_CREAT | O_WRONLY);
    if (fdAgFileData < 0) {
      perror("Erro no lseek na função mandaAgregar.");
      close(fdVendas);
  }
    
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

            int filhoFicheiro1 = open("0.txt", O_RDONLY);
            int filhoFicheiro2 = open("1.txt", O_RDONLY);      

            while((byteslidos=read(filhoFicheiro1,bufferino,tamVendas))>0){
              printf("%s\n", bufferino);
              bufferino[byteslidos-1]='\n';
              bufferino[byteslidos]='\0';
              if(mywrite(pf[1],bufferino,byteslidos)<0) {
                  perror("Erro na escrita do ficheiro vendas para o pipe.");
              }
              posicao++;
            }
            while((byteslidos=read(filhoFicheiro2,bufferino2,tamVendas))>0){
              bufferino2[byteslidos-1]='\n';
              bufferino2[byteslidos]='\0';
              if(write(pf[1],bufferino2,byteslidos)<0) {
                  perror("Erro na escrita do ficheiro vendas para o pipe.");
              }
              posicao++;
            }
             

        
                      
          close(pf[1]);
          close(filhoFicheiro1);
            close(filhoFicheiro2);
    }



return posicao;

}

void agrega(int ii){
    char posicaoSI[21];
    char posicaoSN[21];
    int byteslidos, posicaoI, posicaoN=0;
    int fdPosAgr = myopen("posAgregador", O_CREAT | O_RDWR);
    int tamFicheiro = fsize("vendas");

        //ver se o ficheiro está vazio, se estiver agregar a partir da posicao 0
        // escrever o numero de linhas lidas no ficheiro posAgregador
        if((byteslidos=myread(fdPosAgr,posicaoSI,1))==0){
           
            posicaoI = mandaAgregar(0, ii);

            printf("POSICAO I É: %d\n", posicaoI);
                  
            int qtos = sprintf(posicaoSI,"%d\n",posicaoI);

            int qtos2= strlen(posicaoSI);
            int nbw=mywrite(fdPosAgr,posicaoSI, qtos2);
                 
            close(fdPosAgr);
            return;
        }
        //se não estiver
        // ler do ficheiro posAgregador a ultima linha que leu
        // e passa-la como argumento à mandaAgregar
        // por fim, escrever o numero da linha no ficheiro
        else{
                   
          lseek(fdPosAgr,0,SEEK_SET);//coloca a ler desde o inicio o ficheiro poAgr
          int nbr=readline(fdPosAgr,posicaoSN,1);
          close(fdPosAgr);

          if(ii == 1) {
            printf("SOU O PAI: %d\n", ii);
            fdPosAgr = myopen("posAgregador", O_TRUNC | O_RDWR);
            sscanf(posicaoSN,"%d",&posicaoN);
            printf("O PAI VAI COM BYTES: %d\n", (posicaoN*tamVendas)+(((tamFicheiro - (posicaoN*tamVendas))/tamVendas)/2)*tamVendas);
            posicaoN = mandaAgregar((posicaoN*tamVendas)+(((tamFicheiro - (posicaoN*tamVendas))/tamVendas)/2)*tamVendas, ii);
            posicaoN = tamFicheiro / tamVendas;
            //lseek(fdPosAgr,0,SEEK_SET);//coloca a ler desde o inicio o ficheiro poAgr para escrever a nova linha

            sprintf(posicaoSN,"%d",posicaoN);
                 
            int qtos = strlen(posicaoSN);
            mywrite(fdPosAgr,posicaoSN,qtos);
                  
            close(fdPosAgr);
          }
                              
          else {

            printf("SOU O FILHO: %d\n", 0);
            sscanf(posicaoSN,"%d",&posicaoN);
                            
            posicaoN = mandaAgregar(posicaoN*tamVendas, ii);

            sprintf(posicaoSN,"%d",posicaoN);

          }
        }
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

   DEBUG_MACRO("%d\n", conta);
   
   return conta;
}

void verificaAlteracaoCache(visitado artigosVisitados[]) {

  int fd = open("precosAlterados.txt", O_RDONLY);
  int linha;
  float precoAntigo;
  int existe;

  char buffer[2048];

  size_t byteslidos = read(fd, buffer, sizeof(int) + sizeof(float) + 1);

  if(fd < 0)
    return;

  if(byteslidos == 0) {

    close(fd);

    return;
  }
  else {

    readline(fd, buffer, 1);

    sscanf(buffer, "%d %f", &linha, &precoAntigo);

    existe = existeProdutoMaisVisitado(linha, artigosVisitados);

    if(existe >= 0) {

      (*artigosVisitados[existe]).precoArtigo = precoAntigo;

    }

    close(fd);

    fd = open("precosAlterados.txt", O_TRUNC);
    close(fd);

  }

}

/*
Função que executa os comandos necessários para atualizar o stock e
mostrá-lo no stdout ou apenas mostrar no stdout stock e preço, consoante
sejam passados no stdin, respetivamente, o código do artigo e a quantidade
(a inserir em stock ou a vender) ou somente o código do artigo.
*/
void processaComandos(char buffer[], char *comandos, int fdCliente, visitado artigosVisitados[]) {
  int conta, stock, qtos, sinal = 1;
  char codigoArt[700];
  codigoArt[0] = 0;
  char quant[700];
  quant[0] = 0;
  int ii, p;

  conta = divideComandos(comandos, codigoArt, quant);

  DEBUG_MACRO("O codigo a %s, qutd de a: %s\n", codigoArt, quant );

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
  else if (conta == 1){
  		if (codigoArt[0]== 'a'){
   			DEBUG_MACRO("estou a agregar\n");
        p = fork();
        if(p == 0){
          agrega(0);
          exit(0);
        }
        else {
          agrega(1);
        }
        
  			}

  		else{
          verificaAlteracaoCache(artigosVisitados);
   				getStock_Preco(codigoArt, fdCliente, artigosVisitados);
   				DEBUG_MACRO("estou aqui\n");
   			}

	}
	
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
  visitado artigosVisitados[tamCache];

  inicializaArray(artigosVisitados);
  int tam = fsize("1.txt");
  printf("TAMANHO: %d\n", tam);
  mandaAgregarGeral(0);

  // TODO: ler mais do que um byte de cada vez
  while((byteslidos = myreadServidor(fdComum, buffer, 1)) > 0) {
  		DEBUG_MACRO("SERVIDOR diz: o que estou a ler do pipeComum %s\n",buffer );
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
    		DEBUG_MACRO("SERVIDOR diz: comando que estou a ler pipeComum %s\n",comandos);
			if(strlen(comandos) > PIPE_BUF) {
	        perror("Mensagem superior ao tamanho do pipe.");
					continue;
	    }

	    if(comandos[0] != 'a') { 

    	DEBUG_MACRO("Processo %s Comandos %s\n", processo, comandos);

    	//TODO: manter uma estrutura de dados para saber se o ficheiro está aberto
    	if ((fdCliente = myopen(processo, O_WRONLY)) < 0) {
      	perror("Erro ao abrir o pipe cliente especifico.");
      	_exit(errno);
    	}
    	}

			DEBUG_MACRO("fdCliente %d\n", fdCliente);
			processaComandos(buffer, comandos, fdCliente, artigosVisitados);
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
