#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "debug.h"
#include "aux.h"


/*
Devolve o número de linhas lidas do ficheiro artigos até encontrar
a string str ou, caso não encontre, devolve o número seguinte à última
linha antes do EOF do referido ficheiro.
*/
int contaLinhasArtigos(char *str, int *flag) {
  int byteslidos, fdArt, linha = 0, byte;
  char buffer[2048];
  buffer[0] = 0;
  *flag = -1;

  fdArt = myopen("artigos", O_RDONLY);
  if (fdStr < 0){
    perror("Erro ao abrir o ficheiro strings na função contaLinhasArtigos.");
    _exit(errno);
  }

  if ((byte = lseek (fdStr, 0, SEEK_SET)) < 0) {
    perror("Erro ao fazer lseek na função contaLinhasArtigos.");
    close(fdArt);
    return linha;
  }

  DEBUG_MACRO("Em que byte abriu o artigos %d\n", byte);



  while ((byteslidos = readline(fdArt, buffer, 1)) > 0) {
        linha++;
        DEBUG_MACRO("Linha %d\n", linha);
        DEBUG_MACRO("Byteslidos %d\n", byteslidos);
        DEBUG_MACRO("Conteudo do buffer %s\n", buffer);
        if(((*flag) = strcmp(str, buffer)) == 0) {
        DEBUG_MACRO("Flag a cada iteracao %d\n", *flag);
          break;
        }
  }

  DEBUG_MACRO("Flag na contaLinhasArtigos %d\n", *flag);

  //quando não encontra o nome no ficheiro strings quer inseri-lo na linha seguinte
  if((*flag) != 0) linha++;

  close(fdArt);

  return linha;
}



int main()
{
	
	//abrir ficheiros
	int fdArt = myopen("artigos", O_RDWR);
	int fdStrings = myopen("strings", O_RDWR);
	int fdCompactar = myopen("comp",O_CREAT | O_RDWR | O_APPEND);

	//variaveis
	int nLinhaString = 0;
	char nLinhaStringS[2048];
	nLinhaStringS[0]=0;

	int	nLinhaCompactar = 0;
	int flag;

	int nbytes

	//buffers
	char linhaString[2048];

	//ir a String, ler uma linha e guardar o numero dessa linha

	while((nbytes= readline(fdStrings,linhaString,1))>0){
		linhaString[nbytes]='\n';
		nLinhaString = nLinhaString+1;

		sprintf(nLinhaStringS,"%d",nLinhaString);

		int nLinhaArtigo= contaLinhasArtigos(nLinhaStringS,&flag);

		//se encontrou escreve o ficheiro compactar
		if(flag==0){

			int qts= strlen(linhaString);
			mywrite(fdCompactar,linhaString,qts);
			nLinhaCompactar= nLinhaCompactar+1;
		}
		else{
			nLinhaArtigo= nLinhaArtigo-1;
		}

		//ir ao ficheiro artigos e substituir o numero da linha de um artigo pela nova
		lseek(fdArtigos,0,SEEK_SET);

		while((nbytes= readline(fdArtigos,linhaArtigos,1))>0){

			while(){}
				sscanf(linhaArtigos,"%d %s ",&numero2,&lixo2);
				if(nLinhaArtigo == numero2 ){
					sprintf(l)
				}
			}
		}


	}

	
	



	return 0;
}