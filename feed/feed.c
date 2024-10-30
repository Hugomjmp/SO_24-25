#include "./header/feed.h"


int main (int argc, char* args[]){
    int serverPipe, serverPipeCliente,clientePipe;
    char nomePipe[100];
    ClienteDados cd;
    ThreadFeedData tfd;
    pthread_t tid_recebeMensagens, tid_recebeFecho;
    struct sigaction sa;
    sa.sa_sigaction = userRemovido;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);

    if (argc != 2)
    {
        printf("Tem de indicar o nome do utilizador\n");
        return 20;
    }

    serverPipe = open(SERVER_PIPE,O_WRONLY);
    
    if (serverPipe == -1)
    {
        printf("O servidor manager não se encontra em funcionamento...\n");
        return 19;
    }
    
    strcpy(cd.nome, args[1]);
    cd.PID = getpid();
    snprintf(nomePipe, sizeof(nomePipe), CLIENTE_PIPE"_%d", cd.PID);
    strcpy(cd.clientePipe, nomePipe);
    strcpy(tfd.nomePipe,nomePipe);
    write(serverPipe, &cd, sizeof(ClienteDados)); // envia os dados do feed para o manager

    
    serverPipeCliente = open(SERVER_PIPECLIENTE, O_WRONLY);
    tfd.pipeServerCliente = serverPipeCliente;
    
    mkfifo(nomePipe, 0666); //cria um pipe para cada cliente
    tfd.clientePipe = open(nomePipe, O_RDWR);

    //printf("%s",nomePipe);
    
    //tfd.clientePipe = open(nomePipe, O_RDONLY);
    pthread_create(&tid_recebeMensagens,NULL,trataMensagens,(void *) &tfd);
    pthread_create(&tid_recebeFecho, NULL, trataFecho,(void*) &tfd);


    tfd.continua = 1;
    while(tfd.continua == 1)
    {
        Menu();
        trataComandos(&tfd);
    }
    

    //close(clientePipe);
    close(serverPipe);
    close(serverPipeCliente);
    pthread_join(tid_recebeMensagens,NULL);
    unlink(nomePipe);
    return 1;
}
//tratar de comandos...
void trataComandos(ThreadFeedData *tfd){
    char comando[100], parametro[100], parametro2[100], mensagem[MAX_CARACTER_MENSAGEM];
    char duracao;
    Mensagem msg;
    //fgets(comando, sizeof(comando), stdin);
    //comando[strcspn(comando, "\n")] = 0;
    scanf("%s", comando);
    printf("CMD = '%s'\n", comando);
    if (strcmp(comando,"topics") == 0){ //trata do comando topics
        
        strcpy(msg.tipoMSG, comando);
        printf("FD = %d\n",tfd->pipeServerCliente);

        write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));



        printf("[RECEBI] %s\n",comando);
    }else if(strncmp(comando,"msg", strlen("msg")) == 0){ //trata do comando msg
        scanf("%s", parametro);
        //scanf("%d", &duracao);
        //scanf("%s", mensagem);
        strcpy(msg.tipoMSG, parametro);
        printf("FD = %d\n",tfd->pipeServerCliente);
        
        //working
        write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));


        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"subscribe") == 0){ //trata do comando subscribe
        strcpy(msg.tipoMSG, comando);
        write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"unsubscribe") == 0){ //trata do comando unsubscribe
        strcpy(msg.tipoMSG, comando);
        write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"exit") == 0){ //trata do comando exit
        strcpy(msg.tipoMSG, comando);
        write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));
        printf("[RECEBI] %s\n",comando);
        tfd->continua = 0;
    } else {
       printf("NADA!!!\n");
    }
}

void *trataMensagens(void *tfd_aux){
    ThreadFeedData *tfd = (ThreadFeedData*) tfd_aux;
    ClienteDados cd;
    Resposta rsp;
    
    while (tfd->continua == 1)
    {
        printf("FD DO PIPE_CLIENTE = %d\n", tfd->clientePipe);
        read(tfd->clientePipe, &rsp, sizeof(Resposta));
        printf("resposta do Servidor->%d\n",rsp.tipoResposta);
        fflush(stdout);
        switch (rsp.tipoResposta) {
            case 0: {
                printf("Recebi topicos do server \n");
                printf("\t+-------------------------+\n");
                printf("\t| Topico \t | N Mensagens |\n");
                printf("\t+-------------------------+\n");
                for (int i = 0; i < MAX_TOPICOS; i++)
                {
                    printf("\t| %s \t | %d |\n" ,rsp.tpd[i].nomeTopico, rsp.tpd[i].numMensagem);
                    printf("\t+-------------------------+\n");
                }
                break;
            }
            case 1: {

                break;
            }
            case 98: {
                printf("O utilizador %s foi removido do servidor.\n", rsp.msgRsp);
                break;
            }
            case 99: {


                printf("Servidor foi offline...\n");
                printf("A sair....\n");
                tfd->continua = 0;
                break;
            }
        }


        /*if (rsp.tipoResposta == 0)
        {
            printf("Recebi topicos do server \n");
            printf("\t+-------------------------+\n");
            printf("\t| Topico \t | N Mensagens |\n");
            printf("\t+-------------------------+\n");
            for (int i = 0; i < MAX_TOPICOS; i++)
            {
                printf("\t| %s \t | %d |\n" ,rsp.tpd[i].nomeTopico, rsp.tpd[i].numMensagem);
                printf("\t+-------------------------+\n");
            }
        }*/
        
        //printf("| %d | | %s |", tfd->tpd[1].numMensagem, tfd->tpd->nomeTopico);
       // printf("O utilizador %s foi eliminado da plataforma.\n", cd.nome);
    }
    
}
void *trataFecho(void *tfd){ //trata de dizer ao feed para fechar o tudo...
    ThreadFeedData *tf = (ThreadFeedData*) tfd;
    while (tf->continua == 1)
    {
        printf("ola\n");
        sleep(5);
    }
    
}

//Alertar o utilizador que foi removido
void userRemovido(int sinal, siginfo_t *si, void *u){
    printf("fui chamado com valor '%d'\n",si->si_value.sival_int);
    //COMO IMPLEMENTAR AQUI O CONTINUA????
}


/*TODO*/
//Colocar o Sigque a funcionar...
void Menu(){
    printf("\t\e[0;32m+---------------------------------------------------------------------------------+\e[0m\n");
    printf("\t\e[0;32m|                              FEED - MENU                                        |\e[0m\n");
    printf("\t\e[0;32m+---------------------------------------------------------------------------------+\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mtopics              \e[1;32m- Apresenta os topicos existentes na plataforma.            \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mmsg <t> <dur> <msg> \e[1;32m- Envia mensagens.                                          \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34msubscribe <topico>  \e[1;32m- Passa a receber mensagens do topico.                      \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34munsubscribe <topico>\e[1;32m- Deixa de receber mensagens do topico.                     \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mexit                \e[1;32m- Bloqueia novas mensagens para para o topico selecionado.  \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m+---------------------------------------------------------------------------------+\e[0m\n");
}