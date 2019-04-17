#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define tamArt 16
#define formatoStocks "%7d %7d\n"
#define formatoVendas "%7d %7d %7.2f\n"

void criaFicheiros() {
  int fdStks, fdVds;

  //criar e abrir ficheiro stocks
  fdStks = open("stocks.txt", O_CREAT | O_APPEND, 0600);

  if(fdStks < 0) {
    perror("Erro ao abrir o ficheiro stocks.txt");
    _exit(errno);
  }
  //criar e abrir ficheiro vendas
  fdVds = open("vendas.txt", O_CREAT | O_APPEND, 0600);

  if(fdVds < 0) {
    perror("Erro ao abrir o ficheiro vendas.txt");
    _exit(0);
  }

  close(fdStks);
  close(fdVds);

}

int readline(int fildes, char *buf, int nbytes) {
    int byteslidos, total = 0;

    while((byteslidos = read(fildes, buf, nbytes)) > 0){
      total += byteslidos; //vai contando o total de bytes que lê
      if(*buf == '\n') {
        *buf = 0;
        break; // se o char que está naquela posição do buffer for newline
      }
      buf = buf + byteslidos; //aumenta a posição no buffer onde o caracter vai ser armazenado
    }
    return total;
}

int insereStock(char*codigo, char*quantidade){

	int fdStocks, codigoInt, quantidadeInt, bytesEscritos;
	char stocks[100];

	fdStocks = open("stocks.txt", O_WRONLY | O_APPEND);

	codigoInt = atoi(codigo);
	quantidadeInt= atoi(quantidade);

	int qtos = sprintf(stocks, formatoStocks, codigoInt, quantidadeInt); //converte o codigo e quantidade numa string no formato stocks

	if(qtos < 0) {
    	perror("Erro na função sprintf");
    	_exit(errno);
  	}

  	qtos = strlen(stocks);
  	printf("tamanho do formato %d\n",qtos );

  	bytesEscritos=write(fdStocks,stocks,qtos);

  	return bytesEscritos;
}

int insereVenda(char*codigo, char*quantidade){

	int fdVendas, fdArtigos, codigoInt, quantidadeInt, nbytes, codNome;
	float preco, precoTotalVenda;
	char buff[2048];
	char vendas[100];


	fdVendas = open("vendas.txt", O_WRONLY | O_APPEND);

	codigoInt = atoi(codigo);
	quantidadeInt = abs(atoi(quantidade));

	fdArtigos =open("artigos.txt",O_RDONLY);

	if((nbytes=lseek(fdArtigos, (codigoInt-1)*tamArt, SEEK_SET))<0){

		perror("Erro no lseek");
		_exit(errno);

	}
	
	readline(fdArtigos,buff,tamArt);
	sscanf(buff,"%d %f",&codNome, &preco);

	precoTotalVenda = ((float)quantidadeInt)*preco;


	int qtos = sprintf(vendas, formatoVendas, codigoInt, quantidadeInt, precoTotalVenda);

	if(qtos < 0) {
    	perror("Erro na função sprintf");
    	_exit(errno);
  	}

  	qtos = strlen(vendas);
  	printf("tamanho do formato %d\n",qtos );

  	int bytesEscritos=write(fdVendas,vendas,qtos);

  	return bytesEscritos;
}


/*
int procuraQtd(char* codigo){
	int fdStocks;
	int quantidade = 0;
	char quantidade[2048];

	fdStocks = open("stocks.txt", O_RDONLY);




}*/
 


int main(int argc, char *argv[])
{
	/*
	char buffer[2048];
	char bufferStocks[2048];
	int x;
	int fdPipeCVtoSV = open("pipeCVtoSV.txt",O_RDONLY);
	int fdStcks = open("stoks.txt",O_RDONLY);
	int fdPipeSVtoCV = open("pipeSVtoCV.txt",O_RDONLY)

	while((x = read(fdPipeCVtoSV,buffer,1))>0);

	while( )

		if(strcmp(buffer,bufferStocks)==0){
			while(){
				write(fdPipeSVtoCV,buffer,1);
	}
		}


	}*/

	criaFicheiros();
	if(argv[2][0]=='-'){
		insereVenda(argv[1],argv[2]);
	}
	else {
		insereStock(argv[1],argv[2]);
	}





	return 0;
}