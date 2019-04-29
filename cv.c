#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include "debug.h"
#include "aux.h"


/*
Função que cria um pipe com nome específico para cada
cliente que interaja com o servidor. Esse pipe é nomeado
com base no pid do processo que está a executar.
*/
int criaPipeEspecifico() {
  char buffer[200];
  buffer[0] = 0;
  int fd;

  int pid = getpid();

  sprintf(buffer, "p%d.txt", pid);

  if (mkfifo(buffer, permissoes) < 0) {
    perror("Erro ao criar o pipe cliente especifico.");
    DEBUG_MACRO("Nome do pipe %s\n", buffer);
    _exit(errno);
  }

  if ((fd = open(buffer, O_RDWR)) < 0) {
    perror("Erro ao abir o pipe especifico");
    _exit(errno);
  }

  return fd;
}


/*
Função que abre o pipe para onde todos os clientes escrevem
e do qual o servidor lê.
*/int abrePipeComum() {
  int fd;

  if ((fd = open("pipeComum.txt", O_RDWR)) < 0){
    perror("Erro ao abrir o ficheiro pipeComum.txt");
    _exit(errno);
  }

  return fd;
}

/*
Função que lê do pipe e que imprime o resultado no stdout.
*/
void lePipeEspecifico(int fd) {
  int byteslidos = 0;
  char processo[200];
  processo[0] = 0 ;
  char buffer[1024];
  buffer[0] = 0;


  //imprimir o '\n'
  if ((byteslidos = readline(fd, buffer, 1)) > 0) {
    buffer[byteslidos - 1] = '\n';
    buffer[byteslidos] = 0;
    if(write(1, buffer, byteslidos) < 0) {
      perror("Erro ao ler mensagem do pipe Especifico");
      _exit(errno);
    }
  }
}

/*
Função que implementa a rotina do cliente, ou seja, escreve
para o pipe comum o que lê do stdin e que lê do pipe especificamente
criado para o servidor responder a um determinado cliente,
sendo que essas resposta são impressas no stdout (lePipeEspecifico).
*/
void cliente(int fdComum, int fdEspecifico){
  char buffer[1024];
  buffer[0] = 0;
  char aux[1024];
  aux[0] = 0;
  int byteslidos = 0;

  int pid = getpid();

  //lê do stdin para o buffer enqto não for EOF  TODO:PIPE_BUF
  while(byteslidos != EOF) {   //TODO: ler mais do que um byte

    if ((byteslidos = readline(0, buffer, 1)) < 0)
      break;

    //tratamento do pid do processo
    sprintf(aux, "p%d@", pid);

    buffer[byteslidos - 1] = '\n';
    buffer[byteslidos] = 0;

    strcat(aux, buffer);

    DEBUG_MACRO("Aux %s\n", aux);

    int qtos = strlen(aux);

    //escreve para o pipe comum o pid do processo e a mensagem
    if (write(fdComum, aux, qtos) < 0){
        perror("Erro ao escrever pid e mensagem no pipeComum.txt");
        _exit(errno);
    }

    lePipeEspecifico(fdEspecifico);
  }
}

/*
Função que fecha o pipe para onde todos os clientes escrevem
e do qual o servidor lê.
*/
void fechaPipeComum(int fd) {
  if(close(fd) < 0) {
    perror("Erro ao fechar o pipe Comum.");
    _exit(errno);
  }
}

/*
Função que fecha o pipe do processo específico que está
a comunicar com o servidor.
*/
void fechaPipeEspecifico(int fd) {
  if(close(fd) < 0) {
    perror("Erro ao fechar o pipe específico.");
    _exit(errno);
  }
}


//main
int main(int argc, char *argv[]) {

  int fdEspecifico = criaPipeEspecifico();

  int fdComum = abrePipeComum();

  cliente(fdComum, fdEspecifico);

  fechaPipeComum(fdComum);

  fechaPipeEspecifico(fdEspecifico);

  //printf("%d\n", PIPE_BUF);

	return 0;
}
