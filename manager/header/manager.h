//Bibliotecas...

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>




//defines
#define MAX_USERS 10
#define MAX_TOPICOS 20
#define MAX_MSG_PERSISTENTES 5
#define SERVER_PIPE "../SERVER"

//estruturas...
typedef struct
{
    char* nome[100];
    int PID;

}ClienteDados;

typedef struct
{
    char* nome[100];
    int estado;
}Clientes;

typedef struct
{
    //int pipe;
    ClienteDados *cd[MAX_USERS];
    int continua;
}ThreadData;




void Menu();
void trataComandos(ThreadData *td);
void mostraClientes(ThreadData *td);
void incializaTabelaClientes(ClienteDados *cd);

//threads
void *trataClientes(void *cdp);