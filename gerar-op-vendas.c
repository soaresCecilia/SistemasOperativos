#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"gerar-dados.h"

int main(int argv, char **argc) {
  int size_nome = get_size_nome(); 
  int size_preco = get_size_preco(); 

  assert(size_nome == size_preco);

  int num_max = 1;
  if (argv > 1) {
    num_max = atoi(argc[1]);
  }

  for (int it = 0; it < num_max ; it++) {
    int idx = rand() % size_nome;
    int qtd = rand() % 100;
    int sinal = rand() % 2;

    printf("%d\n", idx);
    if (sinal == 0) {
      printf("%d %d\n", idx, qtd);
    }
    else {
      printf("%d -%d\n", idx, qtd);
    }
  }
  return 0;
}
