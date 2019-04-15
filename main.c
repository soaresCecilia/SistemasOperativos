#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "ma.h"


int main(int argc, char *argv[]) {

  if(argc < 4) {
    perror("Número de argumentos insuficientes. Tem de passar 4 argumentos.");
    _exit(0);
  }

  if(argc == 4 && (strcmp(argv[1], "i") == 0)){
      criaFicheiros();
      insereArtigo(argv[3], argv[2]);

  }

  return 0;
}
