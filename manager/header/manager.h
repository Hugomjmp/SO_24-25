//Bibliotecas...

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>


//defines
#define MAX_USERS 10
#define MAX_TOPICOS 20
#define MAX_LINHAS_TOPICOS 100
#define MAX_MSG_PERSISTENTES 5
#define MAX_CARACTER_TOPICO 20
#define MAX_CARACTER_MENSAGEM 300
#define SERVER_PIPE "../SERVER"
#define CLIENTE_PIPE "../CLIENTE"
#define SERVER_PIPECLIENTE "../SERVERCLIENTE"
//#define FILENAME "../mensagens.txt"
//estruturas...

//ESTRUTURA COM INFORMAÇÃO DOS CLIENTES
typedef struct
{
    char nome[100];
    int PID;
    char clientePipe[100];
}ClienteDados;
typedef struct {
    char topico[MAX_CARACTER_TOPICO];
    char userSubscrito[50];
}Subscribe;

//ESTRUTURA COM INFORMAÇÃO DOS TOPICOS
typedef struct {
    char topico[MAX_CARACTER_TOPICO];       //Nome dos tópicos
    char mensagem[MAX_CARACTER_MENSAGEM];   //Mensagem para os tópicos
    int nMensagem;                          //Número de Mensagens para o tópico
    int duracao;                            //Duração das mensagens persistentes
    int estados;                            //Estado 0: Desbloqueado, 1: Bloqueado
    char autor[50];                         //Autor da mensagem
}TopicoTabela;


//ESTRUTURA PARA O CLIENTE INDICAR INFORMAÇÕES SOBRE OS TOPICOS
typedef struct {
    char topico[MAX_CARACTER_TOPICO];
    char mensagem[MAX_CARACTER_MENSAGEM];
    int duracao;
}Topico;

typedef struct
{
    char tipoMSG[100];
    char conteudo[400];
    ClienteDados clienteDados;
    Topico topico;
}Mensagem;


typedef struct
{
    int tipoResposta;
    TopicoTabela topicoTabela[MAX_LINHAS_TOPICOS];
    char msgRsp[100];
}Resposta;

typedef struct
{
    int pipeServer;                                 //NamedPipe do Servidor só para efeitos de "login"
    int pipeServerCliente;                          //NamedPipe onde recebe mensages dos clientes
    ClienteDados cd[MAX_USERS];                     //Array de estrutura com dados dos clientes
    TopicoTabela topicoTabela[MAX_LINHAS_TOPICOS];  //Array de estrutura com dados dos topicos
    int pipeCliente[MAX_USERS];                     //Array de NamedPipes dos clientes
    Subscribe sub[MAX_LINHAS_TOPICOS];              //Array de estrutura das subscrições
    pthread_t tid_Cliente[MAX_USERS];               //Contém os Handles das threads
    pthread_mutex_t *mutex;                         //Contém o mutex para trancar acessos aos dados
    int continua;                                   //Para terminar o programa
    int index;                                      //Auxiliar...
}ThreadData;

typedef struct {
    int index;
    ThreadData *TData;
}ThreadData_extra;


void Menu();
void trataComandos(ThreadData *td);
void mostraClientes(ThreadData *td);
void inicializaTabelaClientes(ThreadData *td);
void mostraTopicos(ThreadData *td);
void mostraMensagens(ThreadData *td,const char* topics);
void inicializaTabelaTopicos(ThreadData *td);
void inicializaTabelaSubscricoes(ThreadData *td);
void respostaTopicos(ThreadData *td, int pipeClienteResp);
//void respostaTopicos(ThreadData_extra *extra, int pipeClienteResp);
void inicializaPipes(ThreadData* td);
void mostraEstados(ThreadData *td);
void bloqueiaTopicos(ThreadData *td,const char* topics);
void desbloqueiaTopicos(ThreadData *td, const char* topics);
void mostraTabela(ThreadData *td);
void mostraSubscribes(ThreadData *td);
void trataCriarSubscriber(ThreadData* td,Mensagem* msg);
void trataRemoverSubscriber(ThreadData *td, Mensagem *msg);
void trataGuardarMensagens(ThreadData *td);
void criaVariavelAmbiente();
void trataLerMensagens(ThreadData *td);

//threads
void *trataClientes(void *cdp);
void *trataComandosCliente(void *td);
//void *trataComandosCliente(void *extra);
void *trataTempoDeVida(void* td);
