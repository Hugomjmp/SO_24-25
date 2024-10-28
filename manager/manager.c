#include "../manager/header/manager.h"



int main(int argc, char* args[]){
    pthread_t tid_trataCliente;
    int serverPipe;
    //ClienteDados cd[MAX_USERS];
    ThreadData td; //= {.cd = cd};
    incializaTabelaClientes(&td);



    // for (int i = 0; i < MAX_USERS; i++)
    // {
        
    // }
    serverPipe = mkfifo(SERVER_PIPE, 0666);
    if (serverPipe == -1)
    {
        printf("[ERRO] Criar o namedpipe...\n");
        return 19;
    }
    serverPipe = open(SERVER_PIPE, O_RDONLY);
    td.pipeServer = serverPipe;
    td.continua = 1;

    pthread_create(&tid_trataCliente,NULL,trataClientes,(void *) &td); //thread para tratar clientes
    

    


    while (td.continua == 1) //tratamento dos comandos
    {
        //printf("%d", td.continua);
        Menu();
        trataComandos(&td);
    }
    
    
    

    close(serverPipe);
    unlink(SERVER_PIPE);
    pthread_join(tid_trataCliente, NULL);
    return 1;

}

void trataComandos(ThreadData* td){
    char comando[100];
    char parametro[100];
    Clientes c;
    union sigval sv; //o mais certo é isto não ficar assim...

    //fgets(comando, sizeof(comando), stdin);
    //comando[strcspn(comando, "\n")] = 0;
    scanf("%s", comando);
    //pthread_create(&th1,NULL,)
    if (strcmp(comando,"users") == 0)
    {
        printf("[RECEBI] %s\n",comando);
        mostraClientes(td);
    }else if(strncmp(comando,"remove", strlen("remove")) == 0){
        scanf("%s", parametro);
        printf("-> Parametro: %s", parametro);

        sv.sival_int = 99;
        for (int i = 0; i < MAX_USERS; i++)
        {
            if (strcmp(parametro,td->cd[i].nome) == 0)
                sigqueue(td->cd[i].PID, SIGUSR1, sv);
        }
        
        
            
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"topics") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"show") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"lock") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"close") == 0){
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



void *trataClientes(void *tdp){
    ThreadData *td = (ThreadData*) tdp;
    ClienteDados cd[MAX_USERS];
    int i = 0;
    //printf("olá sou uma thread \n");
    while (td->continua == 1)
    {
        read(td->pipeServer, &cd[i], sizeof(ClienteDados));
        //printf("RECEBI: '%s' com PID '%d'", cd[i].nome, cd[i].PID);
        fflush(stdout);
        strcpy(td->cd[i].nome,cd[i].nome);
        td->cd[i].PID = cd[i].PID;
        i++; // não esquecer que não pode passar do max_users
    }
    
}


void mostraClientes(ThreadData *td){
    printf("\t+----------------------+\n");
    printf("\t| Nome \t | Process ID |\n");
    printf("\t+----------------------+\n");
    for (int i = 0; i < MAX_USERS; i++)
    {
        printf("\t| %s \t | %d |\n" ,td->cd[i].nome, td->cd[i].PID);
        printf("\t+----------------------+\n");
    }
}

void incializaTabelaClientes(ThreadData *td){

for (int i = 0; i < MAX_USERS; i++)
{
    td->cd[i].PID = 0;
    strcpy(td->cd[i].nome,"-1");
}


}