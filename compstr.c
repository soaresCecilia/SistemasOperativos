#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "debug.h"
#include "aux.h"

/*
	funcao que vai ao ficheiro artigos e substituir todos os numeros antigos(nrLinhaStrings)
	pela posicao nova(novoNumeroLinhaCompactar)
*/
void alteraNrArtigo(int fdArtigos, char* novoNumeroLinhaCompactarS, char* nrLinhaStrings){

	int byte;
	char buffer[2048];
	buffer[0] = 0;
   	int byteslidos;
   	int compara=-1;
   	int novoNumeroLinhaCompactar;

   	int linhaNoArtigos=0;

	if ((byte = lseek (fdArtigos, 0, SEEK_SET)) < 0) {
    	perror("Erro ao fazer lseek na função contaLinhasArtigos.");
    	close(fdArtigos);
   		_exit(errno);
  	}

	while ((byteslidos = readline(fdArtigos, buffer, tamArtigo)) > 0) {

       	double precoArtigo=0;
       	char bufferArtigos[2048];
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

		linhaNoArtigos= linhaNoArtigos+1;

		if((compara= strcmp(array[0], nrLinhaStrings))==0){

			sscanf(novoNumeroLinhaCompactarS,"%d",&novoNumeroLinhaCompactar);
			sscanf(array[1],"%lf",&precoArtigo);

  			sprintf(bufferArtigos,formatoArtigo,novoNumeroLinhaCompactar, precoArtigo);
  			int tamBufferArtigos= strlen(bufferArtigos);

  			lseek(fdArtigos,(linhaNoArtigos-1)*tamArtigo,SEEK_SET);
  			mywrite(fdArtigos, bufferArtigos, tamBufferArtigos);
  	}
  }
}

/*
	Função que copiar do ficheiro auxiliar Compactado, para o ficheiro Strings
*/

void toStringTxt(int fdStrings, int fdCompactar){
	int nbytes;
	char buffer[2048];
	buffer[0]=0;

	while((nbytes=readline(fdCompactar,buffer,1))>0){
		buffer[nbytes-1]='\n';
		buffer[nbytes]='\0';
		mywrite(fdStrings,buffer,nbytes);
	}
}

void compactar(int fdArtigos, int fdStrings, int fdCompactar){

	int booleano=-1;

	//
	int nbytes;

	//buffer para onde estou a ler o nome do artigo lido do ficheiro string.txt
	char buffer[2048];
	buffer[0]=0;

	//buffer para passar o numero para uma string
	char nrLinhaStringS[2048];
	nrLinhaStringS[0]=0;

	//inicializar o numero da linha que estamos a ler do ficheiro string.txt
	int nrLinhaString=0;

	int novoNumeroLinhaCompactar=0;
	char novoNumeroLinhaCompactarS[2048];
	novoNumeroLinhaCompactarS[0]=0;

		int byte;
		if ((byte = lseek (fdStrings, 0, SEEK_SET)) < 0) {
    	perror("Erro ao fazer lseek na função contaLinhasArtigos.");
    	close(fdArtigos);
   		_exit(errno);
  	}

		//ler linhas do Strings
	while( (nbytes =readline(fdStrings, buffer,1))>0){


		buffer[nbytes-1] = '\n';
		buffer[nbytes] = 0;

		// nr da linha do artigo no ficheiro strings.txt;
		nrLinhaString= nrLinhaString+1;

		sprintf(nrLinhaStringS,"%d",nrLinhaString);

		// verificar se esse nr da linha existe no artigos.txt
		getExisteLinha (fdArtigos, nrLinhaStringS , &booleano);
		// coloca ponteiro de leitura para ler do inicio do ficheiro artigos, na proxima iteracao do ciclo
		lseek(fdArtigos,0,SEEK_SET);

		if(booleano == 1){
			//funcao escreve no compactar.txt
			int tambuffer= strlen(buffer);
			mywrite(fdCompactar,buffer,tambuffer);
			//numero da linha no compactar onde foi escrito
			novoNumeroLinhaCompactar++;


		sprintf(novoNumeroLinhaCompactarS,"%d",novoNumeroLinhaCompactar);

		// alterar o numero da linha para qual esá a apontar o artigo
		//no ficheiro artigo

		alteraNrArtigo(fdArtigos,novoNumeroLinhaCompactarS, nrLinhaStringS);

		}
	}

	//copiar tudo do ficheiro compactar para o ficheiro String (O_TRUNC)
	close(fdStrings);
	close(fdCompactar);

	fdStrings=myopen("strings", O_WRONLY | O_TRUNC);
	fdCompactar=myopen("compactado",O_RDONLY);

	toStringTxt(fdStrings,fdCompactar);

close(fdArtigos);
close(fdStrings);
close(fdCompactar);
}


int main(){
	//abrir ficheiros
	int fdArtigos = myopen("artigos", O_RDWR);
	int fdStrings = myopen("strings", O_RDWR);
	int fdCompactar = myopen("compactado",O_CREAT | O_RDWR | O_APPEND);

	compactar(fdArtigos,fdStrings, fdCompactar);

return 0;
}
