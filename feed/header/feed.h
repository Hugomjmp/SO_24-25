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
#define MAX_TOPICOS 20
#define MAX_MSG_PERSISTENTES 5
#define MAX_CARACTER_TOPICO 20
#define MAX_LINHAS_TOPICOS 100
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

typedef struct { // a ser usada
    char topico[MAX_CARACTER_TOPICO];       //Nome dos tópicos
    char mensagem[MAX_CARACTER_MENSAGEM];   //Mensagem para os tópicos
    int nMensagem;                          //Número de Mensagens para o tópico
    int duracao;                            //Duração das mensagens persistentes
    int estados;                            //Estado 0: Desbloqueado, 1: Bloqueado
}TopicoTabela;



typedef struct {
    char topico[MAX_CARACTER_TOPICO];
    char mensagem[MAX_CARACTER_MENSAGEM];
    int duracao;
}Topico;
typedef struct
{
    int valor;
    int continua;
    int clientePipe;
    int pipeServerCliente;
    char nomePipe[100];
}ThreadFeedData;


typedef struct
{
    char tipoMSG[100];
    char conteudo[400];
    Topico topico;
}Mensagem;

typedef struct
{
    int tipoResposta;
    TopicoTabela topicoTabela[MAX_LINHAS_TOPICOS];
    char msgRsp[100];
}Resposta;


void *trataMensagens(void *tfd_aux);
void *trataFecho(void *tfd);
void Menu();
void trataComandos();
void userRemovido(int valor, siginfo_t *si, void *u);
void mostraTopicosfeed(Resposta *rsp);