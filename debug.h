#ifndef __DEBUG_H__

/**
@file debug.h
Debuggador do programa.
*/

#include <stdio.h>

/**
\brief Macro que habilita o debug.
*/
#define WITH_DEBUG 1

/**
\brief Macro que define o comportamento do debug macro. Caso o debug esteja
habilitado, escreve para o ficheiro uma mensagem. Caso contrário, não faz nada.
*/
#if WITH_DEBUG
  #define DEBUG_MACRO(...) \
  { \
    char debug[500]; \
    sprintf(debug, __VA_ARGS__); \
    escrever_no_log(debug); \
  }
#else
  #define DEBUG_MACRO(...)
#endif

int abrir_log();

void fechar_log();

void escrever_no_log(char msg[]);

#endif
