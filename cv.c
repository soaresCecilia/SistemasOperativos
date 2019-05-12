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
Função que lê do pipe e que imprime o resultado no stdout.
*/
void lePipeEspecifico(int fd) {
  int byteslidos = 0;
  char buffer[1024];
  buffer[0] = 0;


  //imprimir o '\n'
  if ((byteslidos = readline(fd, buffer, 1)) > 0) {
    buffer[byteslidos - 1] = '\n';
    buffer[byteslidos] = 0;
    if(mywrite(STDOUT_FILENO, buffer, byteslidos) < 0) {
      perror("Erro ao ler mensagem do pipe Especifico");
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
  int byteslidos = 1;

  int pid = getpid();

  //lê do stdin para o buffer enqto não for EOF  TODO:PIPE_BUF
  while(byteslidos > 0) {   //TODO: ler mais do que um byte

    if ((byteslidos = readline(STDIN_FILENO, buffer, 1)) <= 0)
      break;

    if(byteslidos > PIPE_BUF) {
        perror("Mensagem superior ao tamanho do pipe.");
        continue;
    }
    //tratamento do pid do processo
    sprintf(aux, "p%d@", pid);

    buffer[byteslidos - 1] = '\n';
    buffer[byteslidos] = 0;

    strcat(aux, buffer);

    DEBUG_MACRO("Aux %s\n", aux);

    int qtos = strlen(aux);

    //escreve para o pipe comum o pid do processo e a mensagem
    if (mywrite(fdComum, aux, qtos) < 0){
        perror("Erro ao escrever pid e mensagem no pipeComum");
    }

    lePipeEspecifico(fdEspecifico);
  }
}


//main
int main() {

  abrir_log("log_cliente");

  const char *files[2] = {"stocks", "vendas"};

  criaFicheiros(files, 2);

  int fdEspecifico = criaPipeEspecifico();

  int fdComum = abrePipeComum();

  cliente(fdComum, fdEspecifico);

  fechaPipeComum(fdComum);

  fechaPipeEspecifico(fdEspecifico);

  removePipeEspecifico();

  return 0;
}
