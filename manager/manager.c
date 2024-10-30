#include "../manager/header/manager.h"


//usar alarm para o tempo de vida das mensagens...
//ver como colocar depois...
//ou usar uma thread para fazer isto.. 
//ver qual a melhor solução depois
int main(int argc, char* args[]){
    pthread_t tid_trataCliente;
    int serverPipe;
    //ClienteDados cd[MAX_USERS];
    ThreadData td; //= {.cd = cd};
    incializaTabelaClientes(&td);
    incializaTabelaTopicos(&td);


    //######################################
    //#                                    #
    //#       CRIAÇÃO NAMEDPIPPES          #
    //#                                    #
    //######################################
    serverPipe = mkfifo(SERVER_PIPE, 0666);
    td.pipeServerCliente = mkfifo(SERVER_PIPECLIENTE, 0666);
    //--------------------------------------
    if (serverPipe == -1)
    {
        printf("[ERRO] Criar o namedpipe...\n");
        return 19;
    }
    serverPipe = open(SERVER_PIPE, O_RDWR);

    td.pipeServer = serverPipe;
    td.continua = 1;
    //######################################
    //#                                    #
    //#         CRIAÇÃO THREADS            #
    //#                                    #
    //######################################
    pthread_create(&tid_trataCliente, 
                    NULL,
                    trataClientes,
                    (void *) &td
                    ); //thread para tratar clientes
    

    

    //--------------------------------------

    while (td.continua == 1) //tratamento dos comandos
    {
        printf("-> %d", td.continua);
        Menu();
        trataComandos(&td);
    }
    
    
    

    close(serverPipe);
    close(td.pipeServerCliente);
    unlink(SERVER_PIPE);
    unlink(SERVER_PIPECLIENTE);
    pthread_join(tid_trataCliente, NULL);
    return 1;

}

void trataComandos(ThreadData* td){
    char comando[100];
    char parametro[100];
    ClienteDados cd;
    Resposta rsp;
    union sigval sv; //o mais certo é isto não ficar assim...

    //fgets(comando, sizeof(comando), stdin);
    //comando[strcspn(comando, "\n")] = 0;
    scanf("%s", comando);
    if (strcmp(comando,"users") == 0)
    {
        printf("[RECEBI] %s\n",comando);
        mostraClientes(td);
    }else if(strncmp(comando,"remove", strlen("remove")) == 0){ //REMOVER UTILIZADOR...
        scanf("%s", parametro);

        for (int i = 0; i < MAX_USERS; i++)
        {
            if (strcmp(parametro,td->cd[i].nome) == 0) {
                rsp.tipoResposta = 99;
                strcpy(rsp.msgRsp,td->cd[i].nome);
                write(td->pipeCliente[i],&rsp.tipoResposta,sizeof(rsp.tipoResposta));
            }
            else {
                rsp.tipoResposta = 98;
                write(td->pipeCliente[i],&rsp,sizeof(Resposta));
                sleep(1);
            }
        }

        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"topics") == 0){
        mostraTopicos(td); //chama função de mostrar os tópicos
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"show") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"lock") == 0){
        printf("[RECEBI] %s\n",comando);
    }else if(strcmp(comando,"close") == 0){
        printf("[RECEBI] %s\n",comando);
        td->continua = 0;
        rsp.tipoResposta = 99;
        int res;
        for(int i = 0; i < MAX_USERS; i++) {
            printf("PIPE DIZ %d",td->pipeCliente[i]);
            fflush(stdout);
            res = write(td->pipeCliente[i], &rsp.tipoResposta,sizeof(rsp.tipoResposta));
            printf("[res] %d\n",res);
        }
        write(td->pipeServer, "", sizeof("")); //só para destrancar o pipe na thread
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
    ThreadData tdn;
    char nomePipe[100];
    int i = 0;
    inicializaPipes(td);
    while (td->continua == 1)
    {
        read(td->pipeServer, &cd[i], sizeof(ClienteDados));
        //printf("RECEBI: '%s' com PID '%d' com pippe '%s'\n", cd[i].nome, cd[i].PID, cd[i].clientePipe);
        fflush(stdout);
        strcpy(td->cd[i].nome,cd[i].nome);
        td->cd[i].PID = cd[i].PID;
        strcpy(td->cd[i].clientePipe, cd[i].clientePipe); // para aparecer na tabela depois
        td->index = i;
        pthread_create(&td->tid_Cliente[i], NULL, trataComandosCliente, (void *) td); 
        i++; // não esquecer que não pode passar do max_users
    }
    
}
void inicializaPipes(ThreadData* td) {
    for (int i = 0; i < MAX_USERS; i++) {
        td->pipeCliente[i] = -1;
    }
}

void *trataComandosCliente(void *td){
    ThreadData *tdC = (ThreadData*) td;
    Mensagem msg;
    int pipe = open(SERVER_PIPECLIENTE, O_RDONLY);

    //int pipeCliente = open();
    int index = tdC->index;
    printf("PIPE CLIENTE: %s\n",tdC->cd[index].clientePipe);
    sleep(1);
    int pipeClienteResp = open(tdC->cd[index].clientePipe, O_WRONLY);
    tdC->pipeCliente[index] = pipeClienteResp;
    printf("FD DO PIPE_CLIENTE = %d\n", pipeClienteResp);
    fflush(stdout);
    while (tdC->continua == 1)
    {
        //printf("\nCHEGUEI ao READ\n");
        read(pipe, &msg, sizeof(Mensagem)); //recebe as mensagens de comando
        printf("THREAD TRATACOMANDOS: %s\n", msg.tipoMSG);
        fflush(stdout);
        if (strcmp("topics",msg.tipoMSG)==0) //trata do comando topics do cliente
        {

            // printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
            // printf("WTF\n");
            // fflush(stdout);
            // //test pipe
            // printf("ANTES DO WRITE...\n");
            // fflush(stdout);
            respostaTopicos(tdC, pipeClienteResp);
            
            // printf("RES: %d", res);
            // fflush(stdout);
            // printf("FD DO PIPE_CLIENTE = %d\n", pipeClienteResp);
            // fflush(stdout);
            //---------------------------------
            //write(); //tratar dos mutex depois
        }else if (strcmp("msg",msg.tipoMSG)==0)
        {
            printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
        }else if (strcmp("subscribe",msg.tipoMSG)==0)
        {
            printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
        }else if (strcmp("unsubscribe",msg.tipoMSG)==0)
        {
            printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
        }else if (strcmp("exit",msg.tipoMSG)==0)
        {
            printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
        }

    }
    
}
void respostaTopicos(ThreadData *td, int pipeClienteResp){
    Resposta rsp;
    rsp.tipoResposta = 0;
    for (int i = 0; i < MAX_TOPICOS; i++)
    {
        rsp.tpd[i].estado = td->topDt[i].estado;
        strcpy(rsp.tpd[i].mensagem,td->topDt[i].mensagem);
        strcpy(rsp.tpd[i].nomeTopico,td->topDt[i].nomeTopico);
        rsp.tpd[i].numMensagem = td->topDt[i].numMensagem;
    }
    
    /*int res = */write(pipeClienteResp,&rsp,sizeof(Resposta));
}
void mostraClientes(ThreadData *td){
    printf("\t\t+------------------+-+------------+-+----------------------+\n");
    printf("\t\t| Nome             | | Process ID | | Pipe                 |\n");
    printf("\t\t+------------------+-+------------+-+----------------------+\n");
    for (int i = 0; i < MAX_USERS; i++)
    {
        printf("\t\t| %-16s | | %-10d | | %-20s |\n" ,td->cd[i].nome, td->cd[i].PID, td->cd[i].clientePipe);
        printf("\t\t+------------------+-+------------+-+----------------------+\n");
    }
}

void mostraTopicos(ThreadData *td){

    printf("\t\t+-------------------------+\n");
    printf("\t\t| Topico \t | N Mensagens |\n");
    printf("\t\t+-------------------------+\n");
    for (int i = 0; i < MAX_TOPICOS; i++)
    {
        printf("\t\t| %s \t | %d |\n" ,td->topDt[i].nomeTopico, td->topDt[i].numMensagem);
        printf("\t\t+-------------------------+\n");
    }

}
void incializaTabelaClientes(ThreadData *td){

    for (int i = 0; i < MAX_USERS; i++)
    {
        td->cd[i].PID = 0;
        strcpy(td->cd[i].nome,"-1");
        strcpy(td->cd[i].clientePipe,"-1");
    }
}
void incializaTabelaTopicos(ThreadData *td){

    for (int i = 0; i < MAX_TOPICOS; i++)
    {
        td->topDt[i].numMensagem = 0;
        strcpy(td->topDt[i].nomeTopico,"-1");
    }
    //strcpy(td->topDt[5].nomeTopico,"ILDA");
}