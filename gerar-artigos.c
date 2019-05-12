#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

const char *nome_base[] = {
  "chocolate",
  "caderno",
  "chavena",
  "mochila",
  "telemóvel",
  "guitarra",
  "bolo",
  "estrado",
  "bicicleta",
  "caneta",
  "lápis",
  "automóvel",
  "prateleira",
  "impressora",
  "caneca"
};

const float preco_base[] = {
  10.00,
  11.00,
  12.00,
  13.00,
  14.00,
  15.00,
  17.00,
  18.00,
  19.00,
  20.00,
  21.00,
  22.00,
  12345678.00,
  24.00,
  25.00
};

int main(int argv, char **argc) {
  int size_nome = sizeof(nome_base)/sizeof(nome_base[0]);
  int size_preco = sizeof(preco_base)/sizeof(preco_base[0]);
  assert(size_nome == size_preco);

  int num = 1;
  if (argv > 1) {
    num = atoi(argc[1]);
  }

  num = num * size_nome;
  for (int it = 0; it < num ; it++) {
    int idx = it % size_nome;
    printf("i %s-%d %7.2f\n", nome_base[idx], it, preco_base[idx]);
  }
  return 0;
}
