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



int criaPipeEspecifico() {
  char buffer[200];
  buffer[0] = 0;
  int fd;

  int pid = getpid();

  sprintf(buffer, "p%d.txt", pid);

  if (mkfifo(buffer, 0600) < 0) {
    perror("Erro ao criar o pipe cliente especifico.");
    printf("Nome do pipe %s\n", buffer);
    _exit(errno);
  }

  if ((fd = open(buffer, O_RDWR)) < 0) {
    perror("Erro ao abir o pipe especifico");
    _exit(errno);
  }

  return fd;
}


int abrePipeComum() {
  int fd;

  if ((fd = open("pipeComum.txt", O_RDWR)) < 0){
    perror("Erro ao abrir o ficheiro pipeComum.txt");
    _exit(errno);
  }

  return fd;
}

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

//escreve no pipe tudo o que leu do STDIN_FILENO
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

    printf("Aux %s\n", aux);

    int qtos = strlen(aux);

    //escreve para o pipe comum o pid do processo e a mensagem
    if (write(fdComum, aux, qtos) < 0){
        perror("Erro ao escrever pid e mensagem no pipeComum.txt");
        _exit(errno);
    }

    lePipeEspecifico(fdEspecifico);
  }
}

void fechaPipeComum(int fd) {
  if(close(fd) < 0) {
    perror("Erro ao fechar o pipe Comum.");
    _exit(errno);
  }
}


void fechaPipeEspecifico(int fd) {
  if(close(fd) < 0) {
    perror("Erro ao fechar o pipe específico.");
    _exit(errno);
  }
}


//main ler do terminal
int main(int argc, char *argv[]) {

  int fdEspecifico = criaPipeEspecifico();

  int fdComum = abrePipeComum();

  cliente(fdComum, fdEspecifico);

  fechaPipeComum(fdComum);

  fechaPipeEspecifico(fdEspecifico);

  //printf("%d\n", PIPE_BUF);

	return 0;
}
