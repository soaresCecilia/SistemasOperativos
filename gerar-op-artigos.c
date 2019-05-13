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
    printf("n %d %s-%d\n", idx, nome_base[idx], it);
    printf("p %d %12.2f\n", idx, preco_base[idx]);
  }
  return 0;
}
