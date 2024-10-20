#include "../manager/header/manager.h"



int main(int argc, char* args[]){
    int serverPipe;
    serverPipe = mkfifo(SERVER_PIPE, 0666);
    if (serverPipe == -1)
    {
        printf("[ERRO] Criar o namedpipe...\n");
        return 19;
        
    }
    serverPipe = open(SERVER_PIPE, O_RDONLY);
    
    while (1) // mudar isto depois para sair com o  close
    {
        Menu();
        trataComandos();
        
    }
    
    



    return 1;

}

void trataComandos(){
    char comando[100];
    Clientes c;
    pthread_t th1;
    fgets(comando, sizeof(comando), stdin);
    //pthread_create(&th1,NULL,)
    if (strcmp(comando,"users"))
    {
        printf("[RECEBI] %s\n",comando);

    }else if(strcmp(comando,"remove")){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"topics")){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"show")){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"lock")){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"close")){
        printf("[RECEBI] %s\n",comando);
    }

}




void Menu(){

    printf("\t\e[0;32m+-------------------------------------------------------------------------------+\e[0m\n");
    printf("\t\e[0;32m|                              MANAGER - MENU                                   |\e[0m\n");
    printf("\t\e[0;32m+-------------------------------------------------------------------------------+\e[0m\n");
    printf("\t\e[0;32m| \e[1;34musers             \e[1;32m- Lista utilizadores ligados.                               \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mremove <username> \e[1;32m- Remove utilizador da plataforma.                          \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mtopics            \e[1;32m- Apresenta os topicos existentes na plataforma.            \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mshow              \e[1;32m- Apresenta todas as mensagens de um topico.                \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mlock <topico>     \e[1;32m- Bloqueia novas mensagens para para o topico selecionado.  \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34munlock <topico>   \e[1;32m- Desbloqueia topico.                                       \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mclose             \e[1;32m- Encerra a plataforma.                                     \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m+-------------------------------------------------------------------------------+\e[0m\n");
    printf("#> ");

}