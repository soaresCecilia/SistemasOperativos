#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


char buffer[2048];


/*
Cria os ficheiros strings.txt e artigo.txt no diretorio onde corremos
o programa.
*/
void criaFicheiros() {
  int fdStr, fdArt;

  fdStr = open("strings.txt", O_CREAT | O_APPEND, 0600);

  if(fdStr < 0) {
    perror("Erro ao abrir o ficheiro strings.txt");
    _exit(errno);
  }

  fdArt = open("artigos.txt", O_CREAT | O_APPEND, 0600);

  if(fdArt < 0) {
    perror("Erro ao abrir o ficheiro");
    _exit(0);
  }

  close(fdStr);
  close(fdArt);

}

/*
Lê uma linha do ficheiro, nbytes de cada vez, devolvendo o número
total de bytes lidos.
*/
int readline(int fildes, char *buf, int nbytes) {
    int byteslidos, total = 0;

    while((byteslidos = read(fildes, buf, nbytes)) > 0){
      total += byteslidos; //vai contando o total de bytes que lê
      if(*buf == '\n') {
        // Remover o \n
        *buf = 0;
        break; // se o char que está naquela posição do buffer for newline
      }
      buf = buf + byteslidos; //aumenta a posição no buffer onde o caracter vai ser armazenado
    }
    return total;
}

/*
Insere um nome de um artigo no ficheiro strings, caso o mesmo não exista.
A inserção é feita um nome por linha do ficheiro.
*/
int insereNome(char *nome){
  int fd, bytesEscritos = 0;
  char *nome_str = malloc(strlen(nome)+ 2);
  strcpy(nome_str, nome);
  char *string = strcat(nome_str, "\n");

  fd = open("strings.txt", O_RDWR | O_APPEND);

  if(fd < 0) {
    perror("Erro ao abrir o ficheiro strings.txt");
    _exit(errno);
  }

  int qtos = strlen(string);

  bytesEscritos = write(fd, string, qtos);

  free(nome_str);

  close(fd);

  return bytesEscritos;
}


/*
Devolve o número de linhas lidas do ficheiro strings.txt até encontrar
a string str ou, caso não encontre, devolve o número seguinte à última
linha antes do EOF do referido ficheiro.
*/
int contaLinhas(char *str, int *flag) {
  int byteslidos, fdStr, linha = 0;
  buffer[0] = 0;
  *flag = -1;

  fdStr = open("strings.txt", O_RDONLY);
  if (fdStr < 0){
    perror("Erro ao abrir o ficheiro strings");
    _exit(errno);
  }

  if (lseek (fdStr, 0, SEEK_SET) < 0) {
    perror("Erro ao fazer lseek");
    _exit(errno);
  }

  while ((byteslidos = readline(fdStr, buffer, 1)) > 0) {
        linha++;
        if(((*flag) = strcmp(str, buffer)) == 0) {
          break;
        }
  }

  if((*flag) != 0) linha++; //quando não encontra o nome no ficheiro strings quer inseri-lo na linha seguinte

  close(fdStr);

  return linha;
}


/*
Função que insere um artigo no ficheiro artigos.txt. É inserido um artigo
por linha, sendo que cada artigo é composto por um inteiro que representa
a linha que identifica o seu nome no ficheiro strings.txt e um float, com
duas casa decimais, que indica o seu preço. A função devolve o número de
bytes escritos no ficheiro artigos.txt.
*/
int insereArtigo(char *preco, char *nome){
  int fd, bytesEscritos, linha;
  int flag = 0;
  char artigo[100];
  buffer[0] = 0;
  double preco_float = atof(preco);

  linha = contaLinhas(nome, &flag);

  if(flag != 0) insereNome(nome);


  int qtos = sprintf(artigo, "%7d %7.2f\n", linha, preco_float); //converte o numero numa string de base decimal

  if(qtos < 0) {
    perror("Erro na função sprintf");
    _exit(errno);
  }

  fd = open("artigos.txt", O_WRONLY | O_APPEND);

  if(fd < 0) {
    perror("Erro ao abrir o ficheiro");
    _exit(0);
  }

  qtos = strlen(artigo);

  bytesEscritos = write(fd, artigo, qtos);

  close(fd);

  return bytesEscritos;

}


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
