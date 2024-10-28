//Bibliotecas...

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>



//defines
#define MAX_USERS 10
#define MAX_TOPICOS 20
#define MAX_MSG_PERSISTENTES 5
#define MAX_CARACTER_TOPICO 20
#define MAX_CARACTER_MENSAGEM 300
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
    int pipeServer;
    ClienteDados cd[MAX_USERS];
    
    int continua;
    
}ThreadData;



void Menu();
void trataComandos(ThreadData *td);
void mostraClientes(ThreadData *td);
void incializaTabelaClientes(ThreadData *td);

//threads
void *trataClientes(void *cdp);