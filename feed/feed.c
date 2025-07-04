#include "./header/feed.h"

int continua = 1;

int main (int argc, char* args[]){
    int serverPipe, serverPipeCliente,clientePipe;
    char nomePipe[100], serverPipes[100];
    ClienteDados cd;
    ThreadFeedData tfd;
    pthread_t tid_recebeMensagens, tid_recebeFecho;

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
    strcpy(tfd.clienteDados.nome,cd.nome);
    strcpy(tfd.nomePipe,nomePipe);
    write(serverPipe, &cd, sizeof(ClienteDados)); // envia os dados do feed para o manager


    snprintf(serverPipes, sizeof(serverPipes), SERVER_PIPECLIENTE"_%d", cd.PID);
    printf("[serverPipes]:%s \n", serverPipes);
    sleep(2);
    serverPipeCliente = open(serverPipes, O_WRONLY);
    if (serverPipeCliente == -1) {
        perror("Erro ao abrir o pipe");
    }
    printf("PIPESERVER DESCRITOR: %d\n",serverPipeCliente);
    tfd.pipeServerCliente = serverPipeCliente;
    /*serverPipeCliente = open(SERVER_PIPECLIENTE, O_WRONLY);
    tfd.pipeServerCliente = serverPipeCliente;*/
    
    mkfifo(nomePipe, 0666); //cria um pipe para cada cliente
    tfd.clientePipe = open(nomePipe, O_RDWR);

    //printf("%s",nomePipe);
    
    //tfd.clientePipe = open(nomePipe, O_RDONLY);
    pthread_create(&tid_recebeMensagens,NULL,trataMensagens,(void *) &tfd);

    //tfd.continua = 1;
    while(/*tfd.continua*/continua == 1)
    {
        Menu();
        trataComandos(&tfd);
        sleep(1);
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
    char comando[100], topico[100], duracao[10], mensagem[MAX_CARACTER_MENSAGEM];
    Mensagem msg;
    int estadoOkay = 1;
    printf("#> ");
    fgets(comando, sizeof(comando), stdin);
    comando[strcspn(comando, "\n")] = 0; //para remover a quebra de linha
    if (comando[0] != '\0') {
        char *resultado = strtok(comando, " "); //para separar as palavras
        strcpy(comando, resultado);
        printf("CMD = '%s'\n", comando);

        if (strcmp(comando,"topics") == 0){ //trata do comando topics
            printf("A enviar o comando topics...\n");
            strcpy(msg.tipoMSG, comando);
            printf("FD = %d\n",tfd->pipeServerCliente);
            printf("'%s'\n",msg.clienteDados.nome);
            write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));

            //printf("[RECEBI] %s\n",comando);
        }else if(strncmp(comando,"msg", strlen("msg")) == 0){ //trata do comando msg

            strcpy(topico, "");
            strcpy(duracao, "");
            strcpy(mensagem, "");

            //estrair topico
            resultado = strtok(NULL, " ");
            if(resultado != NULL) {
                strcpy(topico, resultado);
            }
            //estrair duracao
            resultado = strtok(NULL, " ");
            if(resultado != NULL) {
                strcpy(duracao, resultado);
            }
            printf("%d",estadoOkay);
            //estrair mensagem
            resultado = strtok(NULL, "");
            if(resultado != NULL) {
                strcpy(mensagem, resultado);
            }

            strcpy(msg.tipoMSG, comando);
            strcpy(msg.topico.topico,topico);
            strcpy(msg.clienteDados.nome, tfd->clienteDados.nome);
            msg.topico.duracao = atoi(duracao);

            /*if(msg.topico.duracao > 0) {
                estadoOkay = 1;
            }
            else {
                printf("A duração tem de ser um inteiro.\n");
                estadoOkay = 0;
            }*/
            strcpy(msg.topico.mensagem,mensagem);
            printf("[CLIENTE] %s",msg.clienteDados.nome);
            /*if(estadoOkay == 1) {*/
            write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));
            //}



            //printf("[RECEBI] %s\n",comando);
        }else if(strncmp(comando,"subscribe", strlen("subscribe")) == 0){ //trata do comando subscribe
            strcpy(topico, "");
            strcpy(mensagem, "");


            strcpy(msg.tipoMSG, comando);
            //estrair topico
            resultado = strtok(NULL, " ");
            if(resultado != NULL) {
                strcpy(topico, resultado);
            }
            strcpy(msg.clienteDados.nome, tfd->clienteDados.nome);
            strcpy(msg.topico.topico,topico);
            write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));
            //printf("[RECEBI] %s\n",comando);
        }else if(strcmp(comando,"unsubscribe") == 0){ //trata do comando unsubscribe
            strcpy(msg.tipoMSG, comando);

            //estrair topico
            resultado = strtok(NULL, " ");
            if(resultado != NULL) {
                strcpy(topico, resultado);
            }

            strcpy(msg.clienteDados.nome, tfd->clienteDados.nome);
            strcpy(msg.topico.topico,topico);

            write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));
            printf("[RECEBI] %s\n",comando);
        }else if(strcmp(comando,"exit") == 0){ //trata do comando exit
            strcpy(msg.tipoMSG, comando);
            write(tfd->pipeServerCliente, &msg, sizeof(Mensagem));
            printf("[RECEBI] %s\n",comando);
            continua = 0;
            /*tfd->continua = 0;*/
        } else {
            printf("NADA!!!\n");
        }
    }
}


//THREAD QUE TRATA DA RESPOSTA DO SERVIDOR
void *trataMensagens(void *tfd_aux){
    ThreadFeedData *tfd = (ThreadFeedData*) tfd_aux;
    ClienteDados cd;
    Resposta rsp;
    //incializaTabelaTopicosFeed(&rsp);
    while (/*tfd->continua*/continua == 1)
    {
        printf("FD DO PIPE_CLIENTE = %d\n", tfd->clientePipe);
        read(tfd->clientePipe, &rsp, sizeof(Resposta));
        printf("resposta do Servidor->%d\n",rsp.tipoResposta);
        fflush(stdout);

        switch (rsp.tipoResposta) {
            //DEVOLVE OS TOPICOS PEDIDOS PELO CLIENTE
            case 0: {
                mostraTopicosfeed(&rsp);
                break;
            }
            case 1: {
                printf("O topico encontra-se bloqueado pelo Administrador.\n");
                printf("Tente mais tarde.\n");
                break;
            }
            case 2: {
                printf("AQUIIIII.\n");
                printf("%s\n",rsp.msgRsp);
                break;
            }
            case 3: {
                printf("Já existe um utilizador com este nome.\n");
                continua = 0;
                /*tfd->continua = 0;*/
                break;
            }
            //DIZ QUE HOUVE UM UTILIZADOR QUE FOI REMOVIDO
            case 98: {
                printf("O utilizador %s foi removido do servidor.\n", rsp.msgRsp);
                break;
            }
            //MANDA DESLIGAR O CLIENTE
            case 99: {

                printf("Servidor foi offline...\n");
                printf("A sair....\n");
                //tfd->continua = 0;
                continua=0;
                break;
            }
        }

    }
    
}
//\e[0;35m
void mostraTopicosfeed(Resposta *rsp) {

    printf("\e[0;32m\e[40m+---------------------------------------------------------------------------"
"------------------------------------------------+\e[0m\n");
    printf("\e[0;32m\e[40m| TOPICO               | N_MSG | MENSAGEM 									  | DURACAO |\e[0m\n");
    printf("\e[0;32m\e[40m+---------------------------------------------------------------------------"
           "------------------------------------------------+\e[0m\n");
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++)
    {

        if (strcmp(rsp->topicoTabela[i].topico,"-1") != 0 /*&& strcmp(rsp->topicoTabela[i].mensagem,"-1") != 0*/) {

            if (i >= 0 && strcmp(rsp->topicoTabela[i].topico,rsp->topicoTabela[i-1].topico) != 0) {
                printf("\e[0;32m\e[40m|\e[0m\e[40m\e[1;37m %-20s \e[0;32m\e[40m|\e[0m", rsp->topicoTabela[i].topico);
            }else {
                printf("\e[0;32m\e[40m|\e[0m\e[40m %-20s \e[0;32m\e[40m|\e[0m", "");
            }
            if (i >= 0 && rsp->topicoTabela[i].nMensagem != 0) {
                printf("\e[40m \e[1;37m%-5d \e[0;32m\e[40m|\e[0m", rsp->topicoTabela[i].nMensagem);
            }else {
                printf("\e[40m \e[1;37m%-5s \e[0;32m\e[40m|\e[0m", "");
            }
            printf("\e[40m \e[1;36m%-80s \e[0;32m\e[40m| \e[0m\e[40m\e[1;37m%-7d \e[0;32m\e[40m|\e[0m\n" ,
                    rsp->topicoTabela[i].mensagem,
                    rsp->topicoTabela[i].duracao);

            printf("\e[0;32m\e[40m+---------------------------------------------------------------------------"
                           "------------------------------------------------+\e[0m\n");
        }
    }

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