#include "debug.h"
#include<stdio.h>
#include<errno.h>

/**
@file debug.h
Debuggador do programa.
*/

/**
\brief Variável global.
*/
FILE * fp;

/**
\brief Função que abre um ficheiro de log.
@returns retorna 1 se deu erro e 0 se foi bem sucedido.
*/
int abrir_log() {
  fp = fopen("log.txt", "a");
  if (fp == NULL) {
    printf("erro ao abrir ficheiro do log %d\n", errno);
    return 1;
  }

  return 0;
}

/**
\brief Função que fecha um ficheiro de log.
*/
void fechar_log() {
  fclose(fp);
}

/**
\brief Função que escreve no ficheiro.
@param msg Mensagem a ser escrita.
*/
void escrever_no_log(char msg[]) {
  fputs(msg,fp);  /*ignora o retorno de erro */
  fflush(fp);
}
