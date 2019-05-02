#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>
#include "aux.h"
#include "debug.h"

dados inicializaEstrutura(int tam) {

  dados info = calloc(tam, sizeof(struct agrega));

  return info;
}


//Aumenta a estutura
int aumentaEstrutura(dados *info, int tam){
  int novotam = 2*tam;

  *info = realloc(*info, (sizeof(struct agrega) * novotam));

  for(int i = tam; i < novotam; i++) {
    (*info)[i].quantidade = 0;
    (*info)[i].preco = 0.0;
  }

  return novotam;
}


void agregarVendas(dados info, int *tam) {
  char buffer[2048];
  buffer[0] = 0;
  int byteslidos = 0,cod, quant;
  float montante;


    //TODO: ler mais do que 1 byte
    while ((byteslidos = readline(0, buffer, 1)) > 0) {
        buffer[byteslidos - 1] = 0;
        sscanf(buffer, "%d %d %f", &cod, &quant, &montante);
        if(cod >= *tam) {
          *tam = aumentaEstrutura(&info, *tam);
        }
        //quando o código ou a quantidade ou o preco não são válidos não agrega
        if(cod >= 1 && quant >= 0 && montante >= 0.0) {
          info[cod].quantidade += quant;
          info[cod].preco += montante;
        }
    }
}


void escreveStdout(dados info, int tam){
  char buffer[2048];
  buffer[0] = 0;
  int qtos, i = 0;


  for(i = 0; i < tam; i++){

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
  int tam = 3;

  info = inicializaEstrutura(tam);

  agregarVendas(info, &tam);

  escreveStdout(info, tam);

  free(info);


  return 0;
}
