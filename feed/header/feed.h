#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>




//defines
#define SERVER_PIPE "../SERVER"
#define CLIENTE_PIPE "../CLIENTE"
//estruturas...
typedef struct
{
    char nome[100];
    int PID;
}ClienteDados;

typedef struct
{
    int valor;
    int continua;
}ThreadFeedData;


//struct sinais .... ver se  é necessário...
typedef struct
{
    
    int sinal;
}SinalData;



void Menu();
void trataComandos();
void userRemovido(int valor, siginfo_t *si, void *u);