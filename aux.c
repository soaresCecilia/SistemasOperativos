#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "debug.h"
#include "aux.h"


/*
Função que ignora os erros EAGAIN e EINTR quando é chamada a system
call read.
A função retorna o número de bytes lidos.
*/
int myread(int fildes, void *buffer, int total_bytes) {
  int total_bytes_lidos = 0;
  int bytes_lidos = 0;
  void *buffer_atual = buffer;
  int ler_bytes = total_bytes;
  int sv_errno = 0;

  while(TRUE){
    bytes_lidos = read(fildes, buffer_atual, ler_bytes);
    sv_errno = errno;

    if (bytes_lidos >= 0) {
      ler_bytes -= bytes_lidos;
      buffer_atual += bytes_lidos;
      total_bytes_lidos += bytes_lidos;
    }

    if (ler_bytes == 0 || bytes_lidos == 0) {
        return total_bytes_lidos;
    }
    else if (sv_errno != EAGAIN && sv_errno != EINTR) {
      return bytes_lidos;
    }
  }
}


/*
Função que ignora os erros EAGAIN e EINTR quando é chamada a system
call open.
A função retorna o descritor do ficheiro.
*/
int myopen(const char *path, int oflag) {
    int fd;

    while(TRUE) {

        if ((oflag & O_CREAT) != 0)
          fd = open(path, oflag, PERMISSOES);
        else
          fd = open(path, oflag);

        if (fd >= 0)
          return fd;
        else if (fd < 0 && errno != EAGAIN && errno != EINTR)
          return fd;
    }
}

/*
Função que ignora os erros EAGAIN e EINTR quando é chamada a system
call write.
A função retorna o número de bytes escritos.
*/
int mywrite(int fildes, void *buffer, int total_bytes) {
  int total_bytes_escritos = 0;
  int bytes_escritos = 0;
  void *buffer_atual = buffer;
  int escrever_bytes = total_bytes;
  int sv_errno = 0;

  while(TRUE){
    bytes_escritos = write(fildes, buffer_atual, escrever_bytes);
    sv_errno = errno;

    if (bytes_escritos >= 0) {
      escrever_bytes -= bytes_escritos;
      buffer_atual += bytes_escritos;
      total_bytes_escritos += bytes_escritos;
    }

    if (escrever_bytes == 0 || bytes_escritos == 0) {
        return total_bytes_escritos;
    }
    else if (sv_errno != EAGAIN && sv_errno != EINTR)
      return bytes_escritos;
    }
}

/*
Função que lê uma linha do ficheiro, nbytes de cada vez, devolvendo o número
total de bytes lidos.
*/
int readline(int fildes, char *buf, int nbytes) {
    int byteslidos, total = 0;

    while((byteslidos = myread(fildes, buf, nbytes)) > 0) {
      total += byteslidos; //vai contando o total de bytes que lê
      if(buf[byteslidos - 1] == '\n') {
        buf[byteslidos - 1] = 0;
        break; // se o char que está naquela posição do buffer for newline
      }
      buf = buf + byteslidos; //aumenta a posição no buffer onde o caracter vai ser armazenado
    }

    return (byteslidos > 0) ? total : byteslidos;
}



/*
Cria os ficheiros passados como pârametros para a função
no diretorio onde corremos o programa.
*/
void criaFicheiros(const char *file[], int n) {
  int fd;

  for(int i = 0; i < n; i++) {
    fd = myopen(file[i], O_CREAT);

    if(fd < 0) {
    perror("Erro ao abrir os ficheiros.");
    _exit(errno);
    }

    close(fd);
  }

}

/*
Função que calcula a quantidade do
artigo, cujo código é passado como parâmetro, com base
na informação obtida no ficheiro stocks.
A função retorna a respetiva quantidade do artigo.
*/
int getQuantidade(char *codigo) {
  int bytesLidos, quantidade = 0, codigoArt = 0;
  char buffer[1024];

  int codigoInt = atoi(codigo);

  int fdStocks = myopen("stocks", O_RDONLY);
	if(fdStocks < 0){
	 perror("Erro ao abrir ficheiro stocks");
   return quantidade;
	}

	if (!existeCodigo(fdStocks, codigoInt, tamStocks)) {
    close(fdStocks);
    return quantidade;
  }


  bytesLidos = readline(fdStocks, buffer, tamStocks);
    if(bytesLidos < 0) {
      perror("Erro ao ler dos stocks");
      close(fdStocks);
      return quantidade;
    }

  sscanf(buffer,"%d %d", &codigoArt, &quantidade);

  return quantidade;
}

/*
Função que abre o pipe para onde todos os clientes escrevem
e do qual o servidor lê.
A função retorna o descritor do ficheiro.
*/
int abrePipeComum() {
  int fd;

  if ((fd = myopen("pipeComum", O_WRONLY)) < 0){
    perror("Erro ao abrir o ficheiro pipeComum");
    _exit(errno);
  }

  return fd;
}


/*
Função que fecha o pipe para onde todos os clientes escrevem
e do qual o servidor lê.
*/
void fechaPipeComum(int fd) {
  if(close(fd) < 0) {
    perror("Erro ao fechar o pipe Comum.");
  }
}

/*
Função que cria um pipe com nome específico para cada
cliente que interaja com o servidor. Esse pipe é nomeado
com base no pid do processo que está a executar.
A função retorna o descritor do pipe com nome criado.
*/
int criaPipeEspecifico() {
  char buffer[200];
  buffer[0] = 0;
  int fd;

  int pid = getpid();

  sprintf(buffer, "p%d", pid);

  if (mkfifo(buffer, PERMISSOES) < 0) {
    perror("Erro ao criar o pipe cliente especifico.");
    _exit(errno);
  }

  if ((fd = myopen(buffer, O_RDWR)) < 0) {
    perror("Erro ao abir o pipe especifico");
    _exit(errno);
  }

  return fd;
}

void removePipeEspecifico() {
  char buffer[200];
  int pid = getpid();

  sprintf(buffer, "p%d", pid);
  remove(buffer);
}

/*
Função que fecha o pipe do processo específico que está
a comunicar com o servidor.
*/
void fechaPipeEspecifico(int fd) {
  if(close(fd) < 0) {
    perror("Erro ao fechar o pipe específico.");
    _exit(errno);
  }
}

/*
Função que verifica se o código introduzido existe no ficheiro.
A função retorna 1 se o artigo existe, 0 se o mesmo não existe ou
se ocorreu algum erro.
*/
int existeCodigo(int fd, int codigoInt, int tamformato) {
  int resultado = 0, nbytes, bytesfim;

  if ((bytesfim = lseek(fd, 0, SEEK_END)) < 0) {
    perror("Erro no lseek a partir do fim na função existeCodigo.");
    return 0;
  }

  if ((nbytes = lseek(fd, (codigoInt - 1) * tamformato, SEEK_SET)) < 0) {
    perror("Erro no lseek da posicao do artigo na função existeCodigo.");
    return 0;
  }

  //se o artigo não existir devolve preço 0 o nbytes tem de ser menor que bytesfim não pode ser igual
  if (nbytes < bytesfim) {
    resultado = 1;
  }

  return resultado;
}

/*
 verificar se esse nr da linha existe no artigos.txt,
 caso encontre, devolve 1, sse não encontrar devolve 0 para booleano

*/
void getExisteLinha (int fdArtigos, char *nrLinhaStringS, int *booleano) {

  int byte;
  char buffer[2048];
  buffer[0] = 0;
  int byteslidos=0;
  *booleano = -1;

  if ((byte = lseek (fdArtigos, 0, SEEK_SET)) < 0) {
    perror("Erro ao fazer lseek na função contaLinhasArtigos.");
    close(fdArtigos);
   _exit(errno);
  }

  while ((byteslidos = readline(fdArtigos, buffer, tamArtigo)) > 0) {


    int cmp = -1;
   	int conta = 0;
  	char* array[2];
  	array[0]=0;
  	array[1]=0;

  	const char s[2] =" ";
  	char* token;

  	token= strtok(buffer,s);

  	while(token != NULL){
  		array[conta] = token;
  		conta++;
  		token = strtok(NULL,s);
  	}

  	if ((cmp = strcmp(array[0], nrLinhaStringS)==0)){

  			*booleano = cmp;

  	}
 }
}

void removeCompactado() {
  remove("compactado");
}
