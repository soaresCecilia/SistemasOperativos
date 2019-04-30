#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>
#include "aux.h"
#include "debug.h"

dados inicializaEstrutura() {
  int n = 10000; //tornar dinamico

  dados info = calloc(n, sizeof(struct agrega));

  return info;
}

void agregarVendas(dados info) {
  char buffer[2048];
  buffer[0] = 0;
  int byteslidos = 0,cod, quant;
  float montante;


    //TODO: ler mais do que 1 byte
    while ((byteslidos = readline(STDIN_FILENO, buffer, 1)) > 0) {
        buffer[byteslidos - 1] = 0;
        sscanf(buffer, "%d %d %f", &cod, &quant, &montante);
        info[cod].quantidade += quant;
        info[cod].preco += montante;
        DEBUG_MACRO("Montante %f\n", montante);
        DEBUG_MACRO("Preço %f\n", info[cod].preco);
        DEBUG_MACRO("Quantidade %d\n", info[cod].quantidade);
    }
}


void escreveStdout(dados info){
  char buffer[2048];
  buffer[0] = 0;
  int qtos, i = 0;


  for(i = 0; i < 10000; i++){

    if(info[i].quantidade != 0) {

      sprintf(buffer, formatoVendas, i, info[i].quantidade, info[i].preco);

      qtos = strlen(buffer);

      write(STDOUT_FILENO, buffer, qtos);
    }

  }

}




int main(int argc, char *argv[]) {
  char buffer[2048];
  buffer[0] = 0;
  //int byteslidos = 0;
  dados info = NULL;

  info = inicializaEstrutura();

  agregarVendas(info);

  escreveStdout(info);

  free(info);


  return 0;
}
