#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include "debug.h"
#include "aux.h"

#define AGREGAR_FICHEIRO 1


/*
Insere um nome de um artigo no ficheiro strings, caso o mesmo não exista.
A inserção é feita um nome por linha do ficheiro.
*/
void insereNome(char *nome){
  int fd;
  char *nome_str = malloc(strlen(nome) + 2); //o \0 e o \n
  strcpy(nome_str, nome);
  char *string = strcat(nome_str, "\n");

  fd = myopen("strings", O_RDWR | O_APPEND);

  if(fd < 0) {
    perror("Erro ao abrir o ficheiro strings na função insereNome.");
    _exit(errno);
  }

  int qtos = strlen(string);

  if(mywrite(fd, string, qtos) < 0) {
    perror("Erro ao escrever no ficheiro strings na função insereNome.");
    close(fd);
    _exit(errno);
  }

  free(nome_str);

  close(fd);
}


/*
Devolve o número de linhas lidas do ficheiro strings até encontrar
a string str ou, caso não encontre, devolve o número seguinte à última
linha antes do EOF do referido ficheiro.
*/
int contaLinhasStr(char *str, int *flag) {
  int byteslidos, fdStr, linha = 0, byte;
  char buffer[2048];
  buffer[0] = 0;
  *flag = -1;

  fdStr = myopen("strings", O_RDONLY);
  if (fdStr < 0){
    perror("Erro ao abrir o ficheiro strings na função contaLinhasStr.");
    _exit(errno);
  }

  if ((byte = lseek (fdStr, 0, SEEK_SET)) < 0) {
    perror("Erro ao fazer lseek na função contaLinhasStr.");
    close(fdStr);
    return linha;
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
Função que insere um artigo no ficheiro artigos. É inserido um artigo
por linha, sendo que cada artigo é composto por um inteiro que representa
a linha que identifica o seu nome no ficheiro strings e um float, com
duas casa decimais, que indica o seu preço. A função devolve o código do
artigo.
*/
int insereArtigo(char *preco, char *nome){
  int fd, linha = 0, byte = 0, codigoInt = -1;
  char codigo[100];
  int flag = 0;
  char artigo[100];
  double preco_float = atof(preco);

  linha = contaLinhasStr(nome, &flag);

  if(flag != 0) insereNome(nome);

  sprintf(artigo, formatoArtigo, linha, preco_float);

  fd = myopen("artigos", O_WRONLY | O_APPEND);

  if(fd < 0) {
    perror("Erro ao abrir o ficheiro na função insereArtigo.");
    _exit(errno);
  }

  int qtos = strlen(artigo);

  if (mywrite(fd, artigo, qtos) < 0) {
    perror("Erro ao escrever no ficheiro artigos na função insereArtigo.");
    close(fd);
    return codigoInt;
  }

  if ((byte = lseek (fd, 0, SEEK_CUR)) < 0) {
    perror("Erro ao fazer lseek na função insereArtigo.");
    close(fd);
    return codigoInt;
  }

  codigoInt = byte / tamArtigo;

  sprintf(codigo, "%d\n", codigoInt);

  qtos = strlen(codigo);

  if (mywrite(STDOUT_FILENO, codigo, qtos) < 0) {
    perror("Erro ao escrever no ecran na função insereArtigo.");
    close(fd);
    return codigoInt;
  }

  close(fd);

  return codigoInt;
}

/*
Função que altera a referência do nome de um determinado artigo.
Quando se pretende alterar um nome de um artigo que não existe o
programa ignora essa pretensão.
*/
void alteraNome(char *codigo, char *novoNome) {
  int linhaNome = 0, linhaAntiga = 0;
  int flag = 0;
  char buffer[2048];
  float preco;
  buffer[0] = 0;

  int codigoInt = atoi(codigo);

  DEBUG_MACRO("Codigo %d\n", codigoInt);

  int fdArt = myopen("artigos", O_RDWR);
  if(fdArt < 0){
    perror("Erro ao abrir o ficheiro artigos na função alteraNome.");
    _exit(errno);
  }

  if ((lseek(fdArt,(codigoInt - 1) * tamArtigo , SEEK_SET)) < 0) {
    perror("Erro no lseek a partir do fim na função existeCodigo.");
    return;
  }

  //Se o artigo não existe não se faz nada.
  int codExiste = existeCodigo(fdArt, codigoInt, tamArtigo);

  DEBUG_MACRO("Código existe? %d\n", codExiste);

  if(!codExiste) {
    close(fdArt);
    return;
  }

  // TODO: RESOLVER SÓ LÊ 1 BYTE DE CADA VEZ, se ler tamArtigo dá erro
  readline(fdArt, buffer, 1);

  DEBUG_MACRO("Leu dos artigos Buffer %s\n", buffer);

  sscanf(buffer, "%d %f", &linhaAntiga, &preco);

  buffer[0] = 0;

  //a linha vai ser a referencia do novo nome
  linhaNome = contaLinhasStr(novoNome, &flag);

  //se não existir o nome para que quer mudar tem de o inserir
  if(flag != 0) insereNome(novoNome);

  sprintf(buffer, formatoArtigo, linhaNome, preco);

  //volta ao inicio da linha que quer alterar
  if (lseek (fdArt, (codigoInt-1) * tamArtigo, SEEK_SET) < 0) {
    perror("Erro ao fazer lseek na função alteraNome.");
    close(fdArt);
    return;
  }

  if (mywrite(fdArt, buffer, tamArtigo) < 0) {
    perror("Erro ao escrever em artigos na função alteraNome.");
    close(fdArt);
    return;
  }

  close(fdArt);
}


/*
Função que altera o preço de um artigo no fiheiro artigos.
No caso de se pretender alterar o preco de um artigo cujo código
não existe, o programa não faz qualquer alteração, simplesmente
ignora essa pretensão.
*/
void alteraPreco(char *codigo, char *novoPreco){
  char buffer[2048];
  int linha, nbytes;
  float precoAntigo;
  buffer[0] = 0;


  int codigoInt = atoi(codigo);
  DEBUG_MACRO("Codigo %d\n", codigoInt);

  float preco = atof(novoPreco);

  //não aceita preços negativos
  if(preco < 0) return;

  int fdArt = myopen("artigos", O_RDWR);
  if(fdArt < 0){
    perror("Erro ao abrir o ficheiro artigos na função alteraPreco.");
    _exit(errno);
  }

  //quando o código do artigo não existe não altera o preco
  if (!existeCodigo(fdArt, codigoInt, tamArtigo)) {
    close(fdArt);
    return;
  }

  else {
      //já está na linha do artigo pretendido
      if ((nbytes = lseek (fdArt, (codigoInt-1) * tamArtigo, SEEK_SET)) < 0) {
        perror("Erro ao fazer lseek na função alteraPreco.");
        close(fdArt);
        return;
      }

      DEBUG_MACRO("Quantos bytes leu para achar a linha do artigo %d\n", nbytes);

      // TODO: RESOLVER SÓ LÊ 1 BYTE DE CADA VEZ, se ler tamArtigo dá erro
      if (readline(fdArt, buffer, 1) < 0) {
        perror("Erro ao ler do ficheiro artigos na função alteraPreco.");
        close(fdArt);
        return;
      }

      sscanf(buffer, "%d %f", &linha, &precoAntigo);
      buffer[0]= 0;

      DEBUG_MACRO("Linha %d Preço antigo %f\n", linha, precoAntigo);

      sprintf(buffer, formatoArtigo, linha, preco);

      DEBUG_MACRO("Buffer %s\n", buffer);


      if (lseek(fdArt, (codigoInt-1) * tamArtigo, SEEK_SET) < 0) {
        perror("Erro ao fazer lseek na função alteraPreco.");
        close(fdArt);
        return;
      }

      if (mywrite(fdArt, buffer, tamArtigo) < 0) {
        perror("Erro ao escrever em artigos na função alteraPreco.");
        close(fdArt);
        return;
      }

      close(fdArt);
  }
}

/*
Função que manda executar o agregador, recebendo o numero de bytes lidos
inicialmente, e que atualiza a variavel global
do número de bytes lidos + os bytes lidos anteriormente
*/
int mandaAgregar(int nBytesLidosAGIni){

    int nbytes;
    int posicao = 0;


    int byteslidos;
    char bufferino[2048];
    int fdVendas = myopen("vendas", O_RDONLY);
    if (fdVendas < 0) {
      perror("Erro ao abrir o ficheiro vendas na função mandaAgregar.");
      _exit(errno);
    }

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

    sprintf(dataHora,"%d-%02d-%02dT%02d:%02d:%02d",ptm->tm_year+1900,ptm->tm_mon,ptm->tm_mday, ptm->tm_hour,
           ptm->tm_min, ptm->tm_sec);

    //codigo do  para mandar fazer o agregador
    int pf[2];


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

          #if AGREGAR_FICHEIRO
              if((execlp("./agf", "./agf", NULL))==-1){
                  perror("Erro na execucao do execlp do agf");
                  _exit(errno);
              }
          #else
              if((execlp("./ag","./ag",NULL))==-1){
                  perror("Erro na execucao do execlp do ag");
                  _exit(errno);
              }
          #endif

          _exit(errno);

      default:
          //pai
          //fechar descritor de leitura do pipe por parte do pai
          close(pf[0]);


          //escrever para o pipe
          while((byteslidos=readline(fdVendas,bufferino,1))>0){

            bufferino[byteslidos-1]='\n';
            bufferino[byteslidos]='\0';
            if(mywrite(pf[1],bufferino,byteslidos)<0) {
                perror("Erro na escrita do ficheiro vendas para o pipe.");
            }
            posicao++;
          }
          close(pf[1]);
    }

return posicao;

}

/*
Função que envia um sinal ao servidor de que foi inserido um novo artigo.
*/
void enviaSinalSv(){
  char buffer[2048];
  buffer[0] = 0;
  int pidSv;

  int fdSv = myopen("servidorPid", O_RDONLY);
  if (fdSv < 0) {
    DEBUG_MACRO("Erro ao ler o ficheiro do servidorPid.\n");
    return;
  }

  if (readline(fdSv, buffer, 1) < 0) { //ler mais do que 1 byte;
    DEBUG_MACRO("Erro ao ler a linha do ficheiro servidorPid\n");
    close(fdSv);
    return;
  }

  close(fdSv);

  sscanf(buffer, "%d\n", &pidSv);

  if (kill(pidSv, SIGUSR1) < 0) {
    DEBUG_MACRO("Erro ao enviar sinal");
    return;
  }
}


//main
int main() {
  int byteslidos = 1;
  char buffer[2048];
  buffer[0] = 0;
  char letra[2];
  letra[0] = 0;
  char nome_codigo[1000];
  nome_codigo[0] = 0;
  char preco_nome[500];
  preco_nome[0] = 0;

  const char *files[3] = {"strings", "artigos", "stocks"};

  abrir_log("log_ma");

  criaFicheiros(files, 3);


  DEBUG_MACRO("Comecar a ler do stdin");
  while(byteslidos > 0) {   //TODO: ler mais do que um byte

    //TODO: mais do que 1 byte
    if ((byteslidos = readline(STDIN_FILENO, buffer, 1)) <= 0)
      break;

    buffer[byteslidos - 1] = '\n';

    buffer[byteslidos] = 0;

    sscanf(buffer, "%s %s %s", letra, nome_codigo, preco_nome);

    if(strcmp(letra, "i") == 0) {
      if(strlen(preco_nome) > 7) {
        DEBUG_MACRO("Produto: %s com preço num formato maior do que o permitido: %s.\n", nome_codigo, preco_nome);
      }
      else {
        insereArtigo(preco_nome, nome_codigo);
        enviaSinalSv();
      }
    }

    if(strcmp(letra, "n") == 0) {
      if(strlen(nome_codigo) > 7) {
        DEBUG_MACRO("Código num formato maior do que o permitido: %s.\n", nome_codigo);
      }
      else alteraNome(nome_codigo, preco_nome);
    }

    if(strcmp(letra, "p") == 0){
        if(strlen(preco_nome) > 7 || strlen(nome_codigo) > 7) {
          DEBUG_MACRO("Produto com código: %s com preço num formato superior ao permitido: %s.\n", nome_codigo, preco_nome);
        }
        else alteraPreco(nome_codigo, preco_nome);
    }

    if(strcmp(letra, "a") == 0){ //Aqui tb tem de se ter em atenção que o tamanho do preço não pode exceder

        char posicaoSI[21];
        char posicaoSN[21];
        int byteslidos, posicaoI, posicaoN=0;
        int fdPosAgr = myopen("posAgregador", O_CREAT | O_RDWR);

        //ver se o ficheiro está vazio, se estiver agregar a partir da posicao 0
        // escrever o numero de linhas lidas no ficheiro posAgregador
        if((byteslidos=myread(fdPosAgr,posicaoSI,100))==0){

            posicaoI = mandaAgregar(0);

            int qtos = sprintf(posicaoSI,"%d",posicaoI);

             int nbw=write(fdPosAgr,posicaoSI,qtos);

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

          int qtos = sprintf(posicaoSN,"%d",posicaoN);


          write(fdPosAgr,posicaoSN,qtos);
          close(fdPosAgr);
        }

    }
  }

  fechar_log();

    return 0;
}
