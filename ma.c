#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h> 
#include "debug.h"
#include "aux.h"

//TODO: no altera nome e preco o que acontece quando o artigo
//não existe ou não existe o ficheiro artigos.

//Quando o artigo tem o mesmo nome e preco diferentes o que fazer.


/*
Insere um nome de um artigo no ficheiro strings, caso o mesmo não exista.
A inserção é feita um nome por linha do ficheiro.
*/
int insereNome(char *nome){
  int fd, bytesEscritos = 0;
  char *nome_str = malloc(strlen(nome) + 2); //o \0 e o \n
  strcpy(nome_str, nome);
  char *string = strcat(nome_str, "\n");

  fd = open("strings.txt", O_RDWR | O_APPEND);

  if(fd < 0) {
    perror("Erro ao abrir o ficheiro strings.txt");
    _exit(errno);
  }

  int qtos = strlen(string);

  bytesEscritos = write(fd, string, qtos);

  free(nome_str);

  close(fd);

  return bytesEscritos;
}


/*
Devolve o número de linhas lidas do ficheiro strings.txt até encontrar
a string str ou, caso não encontre, devolve o número seguinte à última
linha antes do EOF do referido ficheiro.
*/
int contaLinhasStr(char *str, int *flag) {
  int byteslidos, fdStr, linha = 0, byte;
  char buffer[2048];
  buffer[0] = 0;
  *flag = -1;

  fdStr = open("strings.txt", O_RDONLY);
  if (fdStr < 0){
    perror("Erro ao abrir o ficheiro strings");
    _exit(errno);
  }

  if ((byte = lseek (fdStr, 0, SEEK_SET)) < 0) {
    perror("Erro ao fazer lseek");
    _exit(errno);
  }

  DEBUG_MACRO("Em que byte abriu o strings %d\n", byte);

  while ((byteslidos = readline(fdStr, buffer, 1)) > 0) {
        linha++;
        DEBUG_MACRO("Linha %d\n", linha);
        DEBUG_MACRO("Byteslidos %d\n", byteslidos);
        DEBUG_MACRO("Conteudo do buffer %s\n", buffer);
        if(((*flag) = strcmp(str, buffer)) == 0) {
        DEBUG_MACRO("Flag a cada iteracao %d\n", *flag);
          break;
        }
  }

  DEBUG_MACRO("Flag na contaLinhasStr %d\n", *flag);

  //quando não encontra o nome no ficheiro strings quer inseri-lo na linha seguinte
  if((*flag) != 0) linha++;

  close(fdStr);

  return linha;
}


/*
Função que insere um artigo no ficheiro artigos.txt. É inserido um artigo
por linha, sendo que cada artigo é composto por um inteiro que representa
a linha que identifica o seu nome no ficheiro strings.txt e um float, com
duas casa decimais, que indica o seu preço. A função devolve o número de
bytes escritos no ficheiro artigos.txt.
Sempre que é inserido um artigo insere-se o mesmo no ficheiro stock com
a quantidade a zero.
*/
int insereArtigo(char *preco, char *nome){
  int fd, bytesEscritos, linha = 0, byte = 0;
  char codigo[100];
  int flag = 0;
  char artigo[100];
  double preco_float = atof(preco);

  linha = contaLinhasStr(nome, &flag);

  if(flag != 0) insereNome(nome);

  sprintf(artigo, formatoArtigo, linha, preco_float);

  DEBUG_MACRO("Tamanho da string artigos %d\n", qtos);

  fd = open("artigos.txt", O_WRONLY | O_APPEND);

  if(fd < 0) {
    perror("Erro ao abrir o ficheiro");
    _exit(0);
  }

  int qtos = strlen(artigo);

  bytesEscritos = write(fd, artigo, qtos);

  if ((byte = lseek (fd, 0, SEEK_CUR)) < 0) {
    perror("Erro ao fazer lseek");
    _exit(errno);
  }

  int codigoInt = byte / tamArtigo;

  sprintf(codigo, "%d\n", codigoInt);

  qtos = strlen(codigo);

  write(STDOUT_FILENO, codigo, qtos);

  //insere no stocks.txt a quantidade a zero
  //insereStock(codigo, "0");

  close(fd);

  return bytesEscritos;

}

/*
Função que altera a referência do nome de um determinado artigo.
Quando se pretende alterar um nome de um artigo que não existe o
programa ignora essa pretensão.
*/
int alteraNome(char *codigo, char *novoNome) {
  int linhaNome = 0, linhaAntiga = 0;
  int bytesEscritos = 0, flag = 0;
  char buffer[2048];
  float preco;
  buffer[0] = 0;

  int codigoInt = atoi(codigo);

  int fdArt = open("artigos.txt", O_RDWR);
  if(fdArt < 0){
    perror("Erro ao abrir o ficheiro artigos");
    _exit(errno);
  }

  //Se o artigo não existe não se faz nada.
  int codExiste = existeCodigo(fdArt, codigoInt);

  if(!codExiste) return bytesEscritos;

  // TODO: RESOLVER SÓ LÊ 1 BYTE DE CADA VEZ, se ler tamArtigo dá erro
  readline(fdArt, buffer, 1);

  sscanf(buffer, "%d %f", &linhaAntiga, &preco);

  buffer[0] = 0;

  //a linha vai ser a referencia do novo nome
  linhaNome = contaLinhasStr(novoNome, &flag);

  //se não existir o nome para que quer mudar tem de o inserir
  if(flag != 0) insereNome(novoNome);

  sprintf(buffer, formatoArtigo, linhaNome, preco);

  //volta ao inicio da linha que quer alterar
  if (lseek (fdArt, (codigoInt-1) * tamArtigo, SEEK_SET) < 0) {
    perror("Erro ao fazer lseek");
    _exit(errno);
  }

  bytesEscritos = write(fdArt, buffer, tamArtigo);

  close(fdArt);

  DEBUG_MACRO("Buffer %s byteslidos %d\n", buffer, byteslidos);
  DEBUG_MACRO("Linha nova %7d\n", linhaNome);
  DEBUG_MACRO("Linha nova %7d preco%7.2f\n", linhaNome, preco);
  DEBUG_MACRO("Flag %d\n", flag);
  DEBUG_MACRO("Linha nova %7d preco%7.2f\n", linhaNome, preco );
  DEBUG_MACRO("linha Antiga%7d Preco%7.2f\n", linhaAntiga, preco);

  return bytesEscritos;
}


/*
Função que altera o preço de um artigo no fiheiro artigos.txt.
No caso de se pretender alterar o preco de um artigo cujo código
não existe, o programa não faz qualquer alteração, simplesmente
ignora essa pretensão.
*/
int alteraPreco(char *codigo, char *novoPreco){
  char buffer[2048];
  int bytesEscritos = 0, linha;
  float precoAntigo;
  buffer[0] = 0;

  int codigoInt = atoi(codigo);

  float preco = atof(novoPreco);

  int fdArt = open("artigos.txt", O_RDWR);
  if(fdArt < 0){
    perror("Erro ao abrir o ficheiro artigos");
    _exit(errno);
  }

  //já está na linha do artigo pretendido
  if (lseek (fdArt, (codigoInt-1) * tamArtigo, SEEK_SET) < 0) {
    perror("Erro ao fazer lseek");
    _exit(errno);
  }

  // TODO: RESOLVER SÓ LÊ 1 BYTE DE CADA VEZ, se ler tamArtigo dá erro
  readline(fdArt, buffer, 1);

  sscanf(buffer, "%d %f", &linha, &precoAntigo);
  buffer[0]= 0;

  sprintf(buffer, formatoArtigo, linha, preco);

  int codExiste = existeCodigo(fdArt, codigoInt);

  //quando o código do artigo não existe não altera o preco
  if(!codExiste) return bytesEscritos;

  bytesEscritos = write(fdArt, buffer, tamArtigo);

  close(fdArt);

  return bytesEscritos;
}

//Função que manda executar o agregador, recebendo o numero de bytes lidos inicialmente, e que atualiza a variavel global
// do número de bytes lidos + os bytes lidos anteriormente


int mandaAgregar(int nBytesLidosAGIni){

    int nbytes;

    int posicao=0;
    
    int byteslidos;
    char bufferino[2048];
    int fdVendas = open("vendas.txt", O_RDONLY);

    if((nbytes = lseek(fdVendas, nBytesLidosAGIni, SEEK_SET) ) < 0){
        perror("Erro no lseek");
        _exit(errno);
    }

    // codigo para criar nome ficheiro com o resultado da agregacao
    char dataHora[100];

    time_t rawtime = time(NULL);
    
    if (rawtime == -1) {
        perror("The time() function failed");
    }
  
    struct tm *ptm = localtime(&rawtime);
    
    if (ptm == NULL) {
       perror("The localtime() function failed");    
    }
    
    sprintf(dataHora,"%d-%02d-%02dT%02d:%02d:%02d.txt",ptm->tm_year+1900,ptm->tm_mon,ptm->tm_mday, ptm->tm_hour, 
           ptm->tm_min, ptm->tm_sec);

    //codigo do  para mandar fazer o agregador
    int pf[2];

    

    int fdAgFileData = open(dataHora, O_CREAT | O_WRONLY | O_TRUNC, permissoes);
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
            posicao++;
          }
          close(pf[1]);

    }
return posicao;
}

//main
int main() {
  int byteslidos=1;
  char buffer[2048];
  buffer[0] = 0;
  char letra[2];
  letra[0] = 0;
  char nome_codigo[1000];
  nome_codigo[0] = 0;
  char preco_nome[500];
  preco_nome[0] = 0;
  const char *files[3] = {"strings.txt", "artigos.txt", "stocks.txt"};
  
  abrir_log();

  while(byteslidos > 0) {   //TODO: ler mais do que um byte

    //TODO: mais do que 1 byte
    if ((byteslidos = readline(0, buffer, 1)) <= 0)
      break;

    buffer[byteslidos - 1] = '\n';

    buffer[byteslidos] = 0;

    sscanf(buffer, "%s %s %s", letra, nome_codigo, preco_nome);

    if(strcmp(letra, "i") == 0){
        criaFicheiros(files, 3);
        insereArtigo(preco_nome, nome_codigo);
    }

    if(strcmp(letra, "n") == 0){
        alteraNome(nome_codigo, preco_nome);
    }

    if(strcmp(letra, "p") == 0){
        alteraPreco(nome_codigo, preco_nome);
    }

    if(strcmp(letra, "a") == 0){
        
        char posicaoSI[21];
        char posicaoSN[21];
        int byteslidos, posicaoI, posicaoN=0;
        int fdPosAgr = open("posAgregador.txt", O_CREAT | O_RDWR , permissoes);

        //ver se o ficheiro está vazio, se estiver agregar a partir da posicao 0
        // escrever o numero de linhas lidas no ficheiro posAgregador
        if((byteslidos=read(fdPosAgr,posicaoSI,100))==0){
                     
            posicaoI = mandaAgregar(0);
                     
            sprintf(posicaoSI,"%d",posicaoI);
             int nbw=write(fdPosAgr,posicaoSI,100);
                    
            close(fdPosAgr);
        }
        //se não estiver
        // ler do ficheiro posAgregador a ultima linha que leu
        // e passa-la como argumento à mandaAgregar
        // por fim, escrever o numero da linha no ficheiro
        else{
                   
          lseek(fdPosAgr,0,SEEK_SET);//coloca a ler desde o inicio o ficheiro poAgr
          int nbr=read(fdPosAgr,posicaoSN,100);
                      
          sscanf(posicaoSN,"%d",&posicaoN);
                     
          posicaoN += mandaAgregar(posicaoN*tamVendas);
                      
          lseek(fdPosAgr,0,SEEK_SET);//coloca a ler desde o inicio o ficheiro poAgr para escrever a nova linha

          sprintf(posicaoSN,"%d",posicaoN);
          write(fdPosAgr,posicaoSN,100);
          close(fdPosAgr);
        }
        
    }
  }

  fechar_log();

    return 0;
}
