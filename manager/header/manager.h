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
#define CLIENTE_PIPE "../CLIENTE"
#define SERVER_PIPECLIENTE "../SERVERCLIENTE"
//estruturas...
typedef struct
{
    char nome[100];
    int PID;
    char clientePipe[100];
}ClienteDados;

typedef struct
{
    char nomeTopico[MAX_CARACTER_TOPICO];
    char mensagem[MAX_CARACTER_MENSAGEM];
    int numMensagem;
    int estado;     //para o bloqueado e/ou desbloqueado
}TopicoData;

typedef struct
{
    int pipeServer;
    int pipeServerCliente;
    ClienteDados cd[MAX_USERS];
    TopicoData topDt[MAX_TOPICOS];
    //int pipeCliente[MAX_USERS];
    pthread_t tid_Cliente[MAX_USERS];
    int continua;
    int index;
}ThreadData;






typedef struct
{
    char tipoMSG[100];
    char conteudo[400];
}Mensagem;



// controlo do feed...
// typedef struct
// {
//     int valor;
//     int continua;
// }ThreadFeedData;


void Menu();
void trataComandos(ThreadData *td);
void mostraClientes(ThreadData *td);
void incializaTabelaClientes(ThreadData *td);
void mostraTopicos(ThreadData *td);
void incializaTabelaTopicos(ThreadData *td);

//threads
void *trataClientes(void *cdp);
void *trataComandosCliente(void *td);
