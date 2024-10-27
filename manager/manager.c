#include "../manager/header/manager.h"



int main(int argc, char* args[]){
    pthread_t tid_trataCliente;
    int serverPipe;
    ClienteDados cd[MAX_USERS];
    
    ThreadData td = {.cd = cd};
    incializaTabelaClientes(cd);



    // for (int i = 0; i < MAX_USERS; i++)
    // {
        
    // }
    
    pthread_create(&tid_trataCliente,NULL,trataClientes,(void *) &cd);
    
    serverPipe = mkfifo(SERVER_PIPE, 0666);

   

    if (serverPipe == -1)
    {
        printf("[ERRO] Criar o namedpipe...\n");
        return 19;
    }
    serverPipe = open(SERVER_PIPE, O_RDONLY);
    td.continua = 1;
    while (td.continua == 1) // mudar isto depois para sair com o close
    {
        printf("%d", td.continua);
        read(serverPipe, &cd[0],sizeof(ClienteDados));
        
        printf("RECEBI: '%s' com PID '%d'", cd[0].nome, cd[0].PID);
        close(serverPipe);
        Menu();
        trataComandos(&td);
        
    

    }
    
    
    

    close(serverPipe);
    pthread_join(tid_trataCliente, NULL);
    return 1;

}

void trataComandos(ThreadData* td){
    char comando[100];
    Clientes c;

    fgets(comando, sizeof(comando), stdin);
    //pthread_create(&th1,NULL,)
    if (strcmp(comando,"users"))
    {
        printf("[RECEBI] %s\n",comando);
        mostraClientes(&td);
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
        td->continua = 0;
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



void *trataClientes(void *cdp){
    ClienteDados *cd = (ClienteDados*) cdp;
    



    
}


void mostraClientes(ThreadData *td){
    printf("\t+----------------------+\n");
    printf("\t| Nome \t | Process ID |\n");
    for (int i = 0; i < MAX_USERS; i++)
    {
        printf("\t| %s | %d |\n" ,td->cd[i]->nome, td->cd[i]->PID);
        printf("\t+----------------------+\n");
    }


}

void incializaTabelaClientes(ClienteDados *cd){

for (int i = 0; i < MAX_USERS; i++)
{
    cd[i].PID = 0;
    strcpy(cd[i].nome,"-1");
}


}