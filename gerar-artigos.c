#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"gerar-dados.h"

int main(int argv, char **argc) {
  int size_nome = get_size_nome();
  int size_preco = get_size_preco();
  assert(size_nome == size_preco);

  int num = 1;
  if (argv > 1) {
    num = atoi(argc[1]);
  }

  num = num * size_nome;
  for (int it = 0; it < num ; it++) {
    int idx = it % size_nome;
    printf("i %s-%d %15.2f\n", nome_base[idx], it, preco_base[idx]);
  }
  return 0;
}
