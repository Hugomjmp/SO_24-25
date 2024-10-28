#include "./header/feed.h"


int main (int argc, char* args[]){
    int serverPipe, clientePipe;
    char nomePipe[100];
    ClienteDados cd;
    ThreadFeedData tfd;

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
    write(serverPipe, &cd, sizeof(ClienteDados));
    

    
    snprintf(nomePipe, sizeof(nomePipe), CLIENTE_PIPE"_%d", cd.PID);
    //printf("%s",nomePipe);
    clientePipe = mkfifo(nomePipe, 0666);
    



    tfd.continua = 1;
    while(tfd.continua == 1)
    {
        Menu();
        trataComandos();
    }
    








    close(clientePipe);
    close(serverPipe);
    unlink(nomePipe);
    return 1;
}
//tratar de comandos...
void trataComandos(){
    char comando[100];
    fgets(comando, sizeof(comando), stdin);
    comando[strcspn(comando, "\n")] = 0;
        if (strcmp(comando,"topics") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"msg") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"subscribe") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"unsubscribe") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"exit") == 0){
        printf("[RECEBI] %s\n",comando);

    }
}

//Alertar o utilizador que foi removido
void userRemovido(int valor, siginfo_t *si, void *u){
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