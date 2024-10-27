#include "./header/feed.h"




int main (int argc, char* args[]){
    int serverPipe;
    
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
    
    

    

    ClienteDados cd;
    strcpy(cd.nome, args[1]);
    cd.PID = getpid();
    
    write(serverPipe, &cd, sizeof(ClienteDados));


    

    Menu();












    return 1;
}


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