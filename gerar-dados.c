#include "gerar-dados.h"

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

const double preco_base[] = {
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
  1234567899.00,
  -24.00,
  25.00
};

int get_size_nome() {
  return sizeof(nome_base)/sizeof(nome_base[0]);
}

int get_size_preco() {
  return sizeof(preco_base)/sizeof(preco_base[0]);
}
