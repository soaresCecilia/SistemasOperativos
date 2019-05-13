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
Função que abre o ficheiro agregador. A função devolve o número do
descritor de ficheiro.
*/
int abreAgregador() {
  int fd = myopen("agregadorFicheiro", O_CREAT | O_TRUNC | O_RDWR);
  if (fd < 0) {
    perror("Erro ao abrir ficheiro agregador");
    _exit(errno);
  }

  return fd;
}

/*
Função que fecha o ficheiro agregador.
*/
void fechaAgregador(int fd) {
  int fecha = close(fd);
  if (fecha < 0) {
    perror("Erro ao fechar ficheiro agregador");
    _exit(errno);
  }

}



/*
Devolve o número de linhas lidas do ficheiro agregador até encontrar
o código do artigo ou, caso não encontre, devolve o número seguinte à última
linha antes do EOF do referido ficheiro.
*/
int contaLinhas(char *codigo, int *flag, int fdAgregador) {
  int byteslidos, linha = 0, byte;
  char quantidadeBuffer[300];
  char precoBuffer[300];
  char codigoBuffer[300];
  char buffer[2048];
  buffer[0] = 0;
  *flag = -1;

  if ((byte = lseek (fdAgregador, 0, SEEK_SET)) < 0) {
    perror("Erro ao fazer lseek na função contaLinhas.");
    close(fdAgregador);
    return linha;
  }

  while ((byteslidos = readline(fdAgregador, buffer, tamVendas)) > 0) {
        linha++;
        sscanf(buffer, "%s %s %s", codigoBuffer, quantidadeBuffer, precoBuffer);
        if(((*flag) = strcmp(codigo, codigoBuffer)) == 0) {
          break;
        }
  }

  //quando não encontra o código no ficheiro quer inseri-lo na linha seguinte
  if((*flag) != 0) linha++;

  return linha;
}


/*
Função que agrega uma venda no ficheiro agregador, caso
o código, a quantidade e o preço sejam todos maiores que 0.
*/
void agregaVenda(char *buffer, int linha, int fdAgregador){
  int codigoV, quantidadeV, codigoA, quantidadeA, n;
  double precoV, precoA;

  sscanf(buffer, "%d %d %lf", &codigoV, &quantidadeV, &precoV);

  if ((n = lseek(fdAgregador, 0, SEEK_SET)) < 0) {
    perror("Erro no lskeek na função agregaVenda1");
    return;
  }

  if (linha != 0) {

      if (lseek(fdAgregador, (linha - 1) * tamVendas, SEEK_SET) < 0) {
        perror("Erro no lseek na função agregaVenda2");
        return;
      }
  }


  if(codigoV >= 1 && quantidadeV > 0 && precoV > 0.00) {

      readline(fdAgregador, buffer, tamVendas);

      sscanf(buffer, "%d %d %lf", &codigoA, &quantidadeA, &precoA);

      quantidadeA = quantidadeV + quantidadeA;

      precoA = precoV + precoA;

      sprintf(buffer, formatoVendas, codigoA, quantidadeA, precoA);



      if (lseek(fdAgregador, (linha - 1) * tamVendas, SEEK_SET) < 0) {
        perror("Erro no lskeek na função agregaVenda4");
        return;
      }

      if (mywrite(fdAgregador, buffer, strlen(buffer)) < 0) {
        perror("Erro a escrever na função agregaVenda");
        _exit(errno);
      }
    }
}

/*
Função que insere uma venda no ficheiro agregador caso o artigo em causa
ainda não tenha mais nenhuma venda anterior para ser agregada.
Também aqui o código, a quantidade e o preço sejam todos maiores que 0.
*/
void insereAgregador(int codigo, int quantidade, double preco, int fdAgregador) {
  char buffer[2048];
  buffer[0] = 0;
  int byte;

  if(lseek(fdAgregador, 0, SEEK_SET) < 0) {
    perror("Erro no lseek da função insereAgregador");
    _exit(errno);
  }

  if(codigo >= 1 && quantidade > 0 && preco > 0.00) {

      if ((byte = lseek(fdAgregador, 0, SEEK_END)) < 0) {
        perror("Erro no lseek da função insereAgregador");
        return;
      }

      sprintf(buffer, formatoVendas, codigo, quantidade, preco);

      write(fdAgregador, buffer, strlen(buffer));
  }

}

void populaFicheiro(int fdAgregador){
  char buffer[2048];
  buffer[0] = 0;
  char codigoA[100];
  codigoA[0] = 0;
  int byteslidos = 1, flag = 1;
  int codigo, quantidadeA, linha = 0;
  double precoA;


  while (byteslidos > 0) {
      if (readline(STDIN_FILENO, buffer, 1) == 0)
          break;

      sscanf(buffer, "%s %d %lf", codigoA, &quantidadeA, &precoA);

      linha = contaLinhas(codigoA, &flag, fdAgregador);


      sscanf(codigoA, "%d", &codigo);
      if(flag != 0)
          insereAgregador(codigo, quantidadeA, precoA, fdAgregador);

      else
          agregaVenda(buffer, linha, fdAgregador);
  }
}


/*
Função que escreve para o stdout a vendas agregadas de cada
artigo no formato código, quantidade total vendida desse artigo e
montante total vendido do referido artigo.
*/
void escreveStdout(int fdAgregador){
  char buffer[2048];
  buffer[0] = 0;
  int byteslidos;


  if(lseek(fdAgregador, 0, SEEK_SET) < 0) {
    perror("Erro no lseek da função escreveStdout");
    _exit(errno);
  }

  while ((byteslidos = readline(fdAgregador, buffer, tamVendas)) > 0) {
      buffer[byteslidos - 1] = '\n';
      buffer[byteslidos] = '\0';
      mywrite(STDOUT_FILENO, buffer, byteslidos);
  }
}

//main
int main() {

  int fd = abreAgregador();

  populaFicheiro(fd);

  escreveStdout(fd);

  fechaAgregador(fd);


  return 0;
}
