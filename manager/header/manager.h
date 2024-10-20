//Bibliotecas...

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



//defines

#define SERVER_PIPE "../SERVER"

//estruturas...

typedef struct
{
    char* nome[100];
    int estado;
}Clientes;




void Menu();
void trataComandos();