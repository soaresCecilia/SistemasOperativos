#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include "debug.h"
#include "aux.h"

/*
+Conta numero total de strings realmente utilizadas
+*/
int stringsUsadas(int totalStrings){

    int nLido = 0, total;

    int fdArtigos= myopen("artigos", O_RDONLY);

    int nrLinhasString = 0;

    char linhasString[2048];
    linhasString[0]=0;

    while(nrLinhasString <= totalStrings){
       int booleano=-1;

       // falta passar nrLinahsString para formato string
       sprintf(linhasString,"%d",nrLinhasString);

       getExisteLinha(fdArtigos, linhasString, &booleano);

       DEBUG_MACRO("nrLinhasString %d \n", nrLinhasString);

       if (booleano == 1){
        nLido = nLido+1;
       }
       lseek(fdArtigos,0,SEEK_SET);

       nrLinhasString++;
    }

    // passar para a media o nLidos
    total = nLido;
    close(fdArtigos);

    DEBUG_MACRO("Total %d\n",total);

    return total;
}

/*
Conta número total de strings, e escreve-os no ficheiro nTotalStrings
*/
int totalStrings(){
    int nbytes, totalStrings = 0;
    char buffer[2048];
    buffer[0]=0;
    int fdStr = myopen("strings", O_RDONLY);

    while((nbytes=readline(fdStr,buffer,1)) > 0) {
      DEBUG_MACRO("Buffer %s\n", buffer);
      totalStrings++;
    }

  close(fdStr);
  return totalStrings;
}

/*
 Função que calcula a percentagem de strings que estão a ser usadas.
*/
void fazMedia(){
  int totalStr;
  int strUsadas;
  int media, status;

  totalStr = totalStrings();

  strUsadas = stringsUsadas(totalStr);

  DEBUG_MACRO("totalStrings  %d e strRealUsadas %d\n", totalStr, strUsadas);

  media = ((strUsadas * 100) / totalStr);

  int pid = fork();

  if (pid == 0) {
    DEBUG_MACRO("media: %d\n",media );
    if(COMPACTA || media <= 80)
        execlp("./compstr","./compstr",NULL);
  }
  else {
    wait(&status);
    removeCompactado();
  }
}

/*
Função que altera o valor do preço no ficheiro precosAlterados.
*/
int alteraPrecoCache(int codigoProduto, double preco) {

  char buffer[2048];
  int bytesescritos = 0;
  buffer[0] = 0;

  int fd = myopen("precosAlterados", O_CREAT | O_WRONLY | O_TRUNC);

  sprintf(buffer, "%d %f", codigoProduto, preco);

  bytesescritos = mywrite(fd, buffer, sizeof(codigoProduto) + sizeof(preco) + 1);

  close(fd);

  return bytesescritos;

}


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
    return;
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
a linha que identifica o seu nome no ficheiro strings e um double, com
duas casa decimais, que indica o seu preço. A função devolve o código do
artigo.
*/
int insereArtigo(char *preco, char *nome){
  int fd, linha = 0, byte = 0, codigoInt = -1;
  char codigo[100];
  int flag = 0;
  char artigo[100];
  double preco_double = 0.00;

  if(*preco == '-') {
    return codigoInt;
  }

  preco_double = atof(preco);

  linha = contaLinhasStr(nome, &flag);

  if(flag != 0) insereNome(nome);

  sprintf(artigo, formatoArtigo, linha, preco_double);

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
  double preco;
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

  readline(fdArt, buffer, tamArtigo);

  DEBUG_MACRO("Leu dos artigos Buffer %s\n", buffer);

  sscanf(buffer, "%d %lf", &linhaAntiga, &preco);

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
  double precoAntigo;
  buffer[0] = 0;


  int codigoInt = atoi(codigo);
  DEBUG_MACRO("Codigo %d\n", codigoInt);

  double preco = atof(novoPreco);

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

      if (readline(fdArt, buffer, tamArtigo) < 0) {
        perror("Erro ao ler do ficheiro artigos na função alteraPreco.");
        close(fdArt);
        return;
      }

      sscanf(buffer, "%d %lf", &linha, &precoAntigo);
      buffer[0]= 0;

      DEBUG_MACRO("Linha %d Preço antigo %f\n", linha, precoAntigo);

      sprintf(buffer, formatoArtigo, linha, preco);

      alteraPrecoCache(codigoInt, preco);

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

/*
Função que passa o comando a (agregar) para o pipe comum
*/
void ma(int fdPipeComum){

  int pid = getpid();
  char letra[100]="a\n";
  char buffer[2048];
  buffer[0]=0;

  sprintf(buffer, "p%d@%s",pid,letra);

  int qts= strlen(buffer);

  mywrite(fdPipeComum,buffer,qts);
  DEBUG_MACRO("o que estou a enviar para o pipe comum %s\n",buffer );
}



/*
Função que manda executar os respetivos comandos do programa de manutenção
de artigos.
*/
void comandosMA(char *buffer) {
  char letra[2];
  letra[0] = 0;
  char nome_codigo[1000];
  nome_codigo[0] = 0;
  char preco_nome[500];
  preco_nome[0] = 0;


  sscanf(buffer, "%s %s %s", letra, nome_codigo, preco_nome);

  if(strcmp(letra, "i") == 0) {
    if(strlen(preco_nome) > tamPreco) {
    }
    else {
      insereArtigo(preco_nome, nome_codigo);
      fazMedia();
      enviaSinalSv();
    }
  }

  if(strcmp(letra, "n") == 0) {
    if(strlen(nome_codigo) > tamCodigo) {
      DEBUG_MACRO("Código num formato maior do que o permitido: %s.\n", nome_codigo);
    }
    else {
      alteraNome(nome_codigo, preco_nome);
      fazMedia();
    }
  }

  if(strcmp(letra, "p") == 0){
      if(strlen(preco_nome) > tamPreco || strlen(nome_codigo) > tamCodigo) {
        DEBUG_MACRO("Produto com código: %s com preço num formato superior ao permitido: %s.\n", nome_codigo, preco_nome);
      }
      else alteraPreco(nome_codigo, preco_nome);
  }

  if(strcmp(letra, "a") == 0){
      int fdPipeComum = myopen("pipeComum", O_WRONLY);
      ma(fdPipeComum);

  }

}



//main
int main() {
  int byteslidos = 1;
  char buffer[2048];
  buffer[0] = 0;


  const char *files[3] = {"strings", "artigos", "stocks"};

  abrir_log("log_ma");

  criaFicheiros(files, 3);


  DEBUG_MACRO("Comecar a ler do stdin");
  while(byteslidos > 0) {

    if ((byteslidos = readline(STDIN_FILENO, buffer, 1)) <= 0)
      break;

    buffer[byteslidos - 1] = '\n';
    buffer[byteslidos] = 0;

    comandosMA(buffer);
  }

  removePipeEspecifico();


  fechar_log();

    return 0;
}
