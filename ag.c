#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>
#include "aux.h"
#include "debug.h"


/*
Função que aloca memória necessária para estutura de dados com o tamanho
tam e inicializa essa mesma estutura com todas as suas posições a zero.
A função retorna um apontador para a referida estrutura.
*/
dados inicializaEstrutura(int tam) {
  return calloc(tam, sizeof(struct agrega));
}


/*
Função que duplica o tamanho da estrutura de dados agrega.
A função devolve o tamanho atual da estrutura.
*/
dados aumentaEstrutura(dados info, int *tam, int cod){
  int novotam = 2 * (*tam);
  novotam = novotam > cod ? novotam : cod + 1;

  dados tmp = realloc(info, (sizeof(struct agrega) * novotam));
  if (tmp == NULL) {
    perror("Erro ao aumentar a estrutura de dados.");
    _exit(1);
  }

  info = tmp;
  for(int i = (*tam); i < novotam; i++) {
    info[i].quantidade = 0;
    info[i].preco = 0.0;
  }
  *tam = novotam;

  return info;
}

/*
Função que agrega as entradas recebidas pelo STDIN
no formato do ficheiro de vendas, até end-of-file, contando
para esse efeito com o auxílio da estrutura agrega que irá
conter em cada uma das suas posições o somatório das quantidades e
dos montantes vendidos do código correspondente ao índice da
referida estutura.
*/
dados agregarVendas(dados info, int *tam) {
  char buffer[2048];
  buffer[0] = 0;
  int byteslidos = 0, cod, quant;
  float montante;

  //TODO: ler mais do que 1 byte
  while ((byteslidos = readline(0, buffer, 1)) > 0) {

      sscanf(buffer, "%d %d %f", &cod, &quant, &montante);
      if(cod >= *tam) {
        info = aumentaEstrutura(info, tam, cod);
      }
      //quando o código ou a quantidade ou o preco não são válidos não agrega
      if(cod >= 1 && quant >= 0 && montante >= 0.0) {
        info[cod].quantidade += quant;
        info[cod].preco += montante;
      }
  }

  return info;
}

/*
Função que escreve para o stdout a vendas agregadas de cada
artigo no formato código, quantidade total vendida desse artigo e
montante total vendido do referido artigo.
*/
void escreveStdout(dados info, int tam){
  char buffer[2048];
  buffer[0] = 0;
  int qtos, i = 0;


  for(i = 0; i < tam; i++){

    if(info[i].quantidade != 0) {

      sprintf(buffer, formatoVendas, i, info[i].quantidade, info[i].preco);

      qtos = strlen(buffer);

      mywrite(STDOUT_FILENO, buffer, qtos);
    }

  }

}



//main
int main() {

  dados info = NULL;
  int tam = 10;

  info = inicializaEstrutura(tam);

  info = agregarVendas(info, &tam);

  escreveStdout(info, tam);

  free(info);


  return 0;
}
