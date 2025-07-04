#include "../manager/header/manager.h"



int main(int argc, char* args[]){
    pthread_t tid_trataCliente, tid_tempoVida;
    int serverPipe;
    //ClienteDados cd[MAX_USERS];
    ThreadData td; //= {.cd = cd};
    inicializaTabelaClientes(&td);
    inicializaTabelaTopicos(&td);
    inicializaTabelaSubscricoes(&td);
    criaVariavelAmbiente();
    trataLerMensagens(&td);
    //######################################
    //#                                    #
    //#       CRIAÇÃO NAMEDPIPPES          #
    //#                                    #
    //######################################
    serverPipe = mkfifo(SERVER_PIPE, 0666);

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
    pthread_create(&tid_tempoVida,
                    NULL,
                    trataTempoDeVida,
                    (void *) &td
                    );

    

    //--------------------------------------
    //######################################
    //#                                    #
    //#         CRIAÇÃO MUTEX              #
    //#                                    #
    //######################################

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    td.mutex = &mutex;


    //--------------------------------------
    while (td.continua == 1) //tratamento dos comandos
    {
        printf("-> %d", td.continua);
        Menu();
        trataComandos(&td);
    }
    
    
    

    close(serverPipe);
    //close(td.pipeServerCliente);
    unlink(SERVER_PIPE);
    unlink(SERVER_PIPECLIENTE);
    pthread_mutex_destroy(td.mutex);
    pthread_join(tid_trataCliente, NULL);
    return 1;

}

void trataComandos(ThreadData* td){
    char comando[100];
    char user[100], topics[MAX_CARACTER_TOPICO];
    ClienteDados cd;
    Resposta rsp;
    union sigval sv; //o mais certo é isto não ficar assim...

    fgets(comando, sizeof(comando), stdin);
    comando[strcspn(comando, "\n")] = 0;
    if (comando[0] != '\0') {
        char *resultado = strtok(comando, " "); //para separar as palavras
        strcpy(comando, resultado);
        //scanf("%s", comando);
        //COMANDO PARA MOSTRAR OS USERS
        if (strcmp(comando,"users") == 0)
        {
            //printf("[RECEBI] %s\n",comando);
            printf("A mostrar os utilizadores..\n");
            mostraClientes(td);
        }
        //COMANDO PARA REMOVER UTILIZADORES
        else if(strncmp(comando,"remove", strlen("remove")) == 0){ //REMOVER UTILIZADOR...

            //estrair topico
            resultado = strtok(NULL, " ");
            if(resultado != NULL) {
                strcpy(user, resultado);
            }
            //scanf("%s", parametro);

            for (int i = 0; i < MAX_USERS; i++)
            {
                if (strcmp(user,td->cd[i].nome) == 0) {
                    rsp.tipoResposta = 99;
                    strcpy(rsp.msgRsp,td->cd[i].nome);
                    write(td->pipeCliente[i],&rsp.tipoResposta,sizeof(rsp.tipoResposta));

                    td->cd[i].PID = 0;
                    strcpy(td->cd[i].nome,"-1");
                    strcpy(td->cd[i].clientePipe,"-1");
                }
                else {
                    rsp.tipoResposta = 98;
                    write(td->pipeCliente[i],&rsp,sizeof(Resposta));
                    sleep(1);
                }
            }

            printf("[RECEBI] %s\n",comando);
        }
        //COMANDO PARA MOSTRAR TOPICO E NUMERO DE MENSAGENS PERSISTENTES
        else if(strcmp(comando,"topics") == 0){
            mostraTopicos(td); //chama função de mostrar os tópicos
            //printf("[RECEBI] %s\n",comando);
        }
        //COMANDO PARA MOSTRAR MENSAGENS DE UM DETERMINADO TOPICO
        else if(strcmp(comando,"show") == 0){
            //estrair topico
            resultado = strtok(NULL, " ");
            if(resultado != NULL) {
                strcpy(topics, resultado);
                mostraMensagens(td,topics);
            }else {
                printf("Tem de introduzir um topico\n");
            }
            //printf("[RECEBI] %s\n",comando);
        }
        //COMANDO PARA BLOQUEADER UM TOPICO
        else if(strcmp(comando,"lock") == 0){
            //estrair topico
            resultado = strtok(NULL, " ");
            if(resultado != NULL) {
                strcpy(topics, resultado);
                bloqueiaTopicos(td,topics);
            }else {
                printf("Tem de introduzir um topico\n");
            }

            //printf("[RECEBI] %s\n",comando);
        }
        //COMANDO PARA DESBLOQUEAR UM TOPICO
        else if(strcmp(comando,"unlock") == 0) {
            //estrair topico
            resultado = strtok(NULL, " ");
            if(resultado != NULL) {
                strcpy(topics, resultado);
                desbloqueiaTopicos(td,topics);
            }else {
                printf("Tem de introduzir um topico\n");
            }
        }
        else if (strcmp(comando,"status") == 0) {
            printf("[RECEBI] %s\n",comando);
            mostraEstados(td);
        }
        else if(strcmp(comando,"tabela") == 0) {
            mostraTabela(td);
        }
        else if(strcmp(comando,"subs") == 0) {
            mostraSubscribes(td);
        }
        //COMANDO PARA TERMINAR O MANAGER E INFORMAR OS FEED'S
        else if(strcmp(comando,"close") == 0){
            td->continua = 0;
            rsp.tipoResposta = 99;
            int res;
            for(int i = 0; i < MAX_USERS; i++) {
                if (td->pipeCliente[i] != -1) { //só faz para os clientes ligados
                    //printf("PIPE DIZ %d",td->pipeCliente[i]);
                    //fflush(stdout);
                    res = write(td->pipeCliente[i], &rsp.tipoResposta,sizeof(rsp.tipoResposta));
                    //printf("[res] %d\n",res);
                }
            }
            trataGuardarMensagens(td);
            write(td->pipeServer, "", sizeof("")); //só para destrancar o pipe na thread
        }
    }

}


void *trataTempoDeVida(void* tdt) {
    ThreadData *td = (ThreadData*)tdt;
    while(td->continua == 1) {
        pthread_mutex_lock(td->mutex);
        for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
            if (strcmp(td->topicoTabela[i].mensagem, "-1") != 0 &&
                td->topicoTabela[i].duracao > 0) {
                    td->topicoTabela[i].duracao--;
            }
            if (strcmp(td->topicoTabela[i].mensagem, "-1") != 0 &&
                td->topicoTabela[i].duracao == 0) {


                strcpy(td->topicoTabela[i].mensagem, "-1"); //remove mensagem
                td->topicoTabela[i].duracao = 0;               //coloca duracao a zero
                if(td->topicoTabela[i].nMensagem > 0) {
                    td->topicoTabela[i].nMensagem--; //Desconta no nMensagens
                }
                printf("apaguei mensagem do topico: %s\n", td->topicoTabela[i].topico);
            }
        }
        pthread_mutex_unlock(td->mutex);
        sleep(1);


    }


}
void mostraTabela(ThreadData *td) { //escondido do menu
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        if (strcmp(td->topicoTabela[i].topico, "-1") != 0) {
            printf("|%s|", td->topicoTabela[i].topico);
            printf("%s|", td->topicoTabela[i].autor);
            printf("%d|", td->topicoTabela[i].nMensagem);
            printf("%s|", td->topicoTabela[i].mensagem);
            printf("%d|", td->topicoTabela[i].duracao);
            printf("%d|\n", td->topicoTabela[i].estados);
        }
    }
    pthread_mutex_unlock(td->mutex);
}
void *trataClientes(void *tdp){
    ThreadData *td = (ThreadData*) tdp;
    ClienteDados cd[MAX_USERS];

    pthread_mutex_t mutex;


    ThreadData_extra extra[MAX_USERS];

    for(int i = 0; i < MAX_USERS; i++) {
        extra[i].index = -1;
    }
    int descritor;
    int i = 0;
    inicializaPipes(td);
    while (td->continua == 1)
    {

        read(td->pipeServer, &cd[i], sizeof(ClienteDados));
        //printf("RECEBI: '%s' com PID '%d' com pippe '%s'\n", cd[i].nome, cd[i].PID, cd[i].clientePipe);
        fflush(stdout);
        pthread_mutex_lock(td->mutex);
        strcpy(td->cd[i].nome,cd[i].nome);                  //recebe o nome
        td->cd[i].PID = cd[i].PID;                          //recebe o pid
        strcpy(td->cd[i].clientePipe, cd[i].clientePipe);   // para aparecer na tabela depois
        //extra[i].index = i;
        //td->index = i;                                      //index
        //descritor = open(td->cd[i].clientePipe,O_WRONLY);
        //td->pipeCliente[i] = descritor;
        extra[i].td = td;
        extra[i].index = i;


        //pthread_create(&td->tid_Cliente[i], NULL, trataComandosCliente, (void *) td);
        pthread_create(&td->tid_Cliente[i], NULL, trataComandosCliente, (void *) &extra[i]);
        /*for (int j = 0; j < MAX_USERS; j++) {
            printf("INDEX %d\n",extra[j].index);
        }*/
        i++;
        /*printf("Tabela dos pipes\n");
            for (int i = 0; i < MAX_USERS; i++) {
                printf("[INDEX CLIENTES]: | %d |\n", extra[i].index);
            }*/
        pthread_mutex_unlock(td->mutex);

// não esquecer que não pode passar do max_users
    }

}
void inicializaPipes(ThreadData* td) {
    for (int i = 0; i < MAX_USERS; i++) {
        td->pipeCliente[i] = -1;
    }
}
void *trataComandosCliente(void *td_extra){
    ThreadData_extra *extra = (ThreadData_extra*) td_extra;
    ThreadData *tdC = extra->td;
    int index = extra->index;
    Mensagem msg;
    Resposta rsp;
    int nMsg = 0;
    char clientePipeName[50];
    char serverPipes[100];
    char resposta[460];
    int subscribe;

    int escreveuMsg;

    snprintf(serverPipes, sizeof(serverPipes), SERVER_PIPECLIENTE"_%d", extra->td->cd[index].PID);
    printf("[SERVER PIPE]:%s\n", serverPipes);
    mkfifo(serverPipes, 0666);

    int pipe = open(serverPipes, O_RDONLY);
    if (pipe == -1) {
        perror("Erro ao abrir o pipe do servidor");
        pthread_exit(NULL);
    }
    tdC->pipeServerCliente[index] = pipe;
    sleep(1);

    const int pipeClienteResp = open(tdC->cd[index].clientePipe, O_WRONLY);
    //const int pipeClienteResp = open(serverPipes, O_WRONLY);
    tdC->pipeCliente[index] = pipeClienteResp;
    if (pipeClienteResp == -1) {
        perror("Erro ao abrir o pipe do cliente");
    }

    fflush(stdout);
    while (tdC->continua == 1)
    {
        //printf("[THREAD %d]",index);
        escreveuMsg = 0;
        subscribe = 0;


        //read(pipe, &msg, sizeof(Mensagem)); //recebe as mensagens de comando
        if (!read(pipe, &msg, sizeof(Mensagem))) {
            break;
        }//recebe as mensagens de comando

        fflush(stdout);
        //TRATA DO COMANDO TOPICS DO CLIENTEe
        if (strcmp("topics",msg.tipoMSG) == 0)
        {
            respostaTopicos(tdC, pipeClienteResp);
        }

        //TRATA DO COMANDO MSG DO CLIENTE
        else if (strcmp("msg",msg.tipoMSG)==0) {
            printf("[RECEBI DO CLIENTE TIPO] %s\n",msg.tipoMSG);
            printf("[RECEBI DO CLIENTE NOMETOPIC] %s\n",msg.topico.topico);
            printf("[RECEBI DO CLIENTE DURACAO] %d\n",msg.topico.duracao);
            printf("[RECEBI DO CLIENTE MENSAGEM] %s\n",msg.topico.mensagem);

            pthread_mutex_lock(tdC->mutex);
            for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
                // para a primeira vez...
                if (strcmp(tdC->topicoTabela[i].topico,"-1") == 0 && escreveuMsg == 0){
                    for (int j = i; j < i+5; j++) {
                        strcpy(tdC->topicoTabela[j].topico,msg.topico.topico);
                    }
                    if (strcmp(tdC->topicoTabela[i].topico,msg.topico.topico) == 0)
                        tdC->topicoTabela[i].nMensagem++; //incremento nMensages
                    if (strcmp(tdC->topicoTabela[i].topico,msg.topico.topico) == 0 &&
                        strcmp(tdC->topicoTabela[i].mensagem,"-1") == 0) {
                        strcpy(tdC->topicoTabela[i].mensagem,msg.topico.mensagem);
                        strcpy(tdC->topicoTabela[i].autor,msg.clienteDados.nome);
                        tdC->topicoTabela[i].duracao = msg.topico.duracao;
                        escreveuMsg = 1;
                        //break;
                    }
                }
                // faz a segunda, terceira.... vez
                if (strcmp(tdC->topicoTabela[i].topico,msg.topico.topico) == 0
                    && escreveuMsg == 0) {
                    if (tdC->topicoTabela[i].estados == 1) {
                        for(int k = 0; k < MAX_USERS; k++) {
                            if(strcmp(tdC->cd[k].nome, msg.clienteDados.nome) == 0) {
                                rsp.tipoResposta = 1;
                                int fd = open(tdC->cd[k].clientePipe, O_WRONLY);
                                write(fd,&rsp,sizeof(Resposta));
                                close(fd);
                            }
                        }
                        break;
                    }
                    //printf("CHEGUEI AQUI!\n");
                    //fflush(stdout);
                    tdC->topicoTabela[i].nMensagem++; //ver isto
                    for (int j = i; j < i + 5; j++) {
                        if (strcmp(tdC->topicoTabela[j].mensagem,"-1") == 0) {
                            strcpy(tdC->topicoTabela[j].mensagem,msg.topico.mensagem);
                            tdC->topicoTabela[j].duracao = msg.topico.duracao;
                            strcpy(tdC->topicoTabela[j].autor,msg.clienteDados.nome);
                            break;
                        }
                    }
                    //strcpy(tdC->topicoTabela[i+1].mensagem,msg.topico.mensagem);
                    tdC->topicoTabela[i+1].duracao = msg.topico.duracao;
                    for (int z = 0; z < MAX_LINHAS_TOPICOS; z++) {
                        if( strcmp(tdC->sub[z].topico,"-1") != 0 &&
                            strcmp(tdC->sub[z].topico,msg.topico.topico) == 0) {
                            printf("TOPICO ENCONTRADO: %s\n", tdC->sub[i].topico);
                            for(int k = 0; k < MAX_USERS; k++) {
                                if (strcmp(tdC->cd[k].nome,tdC->sub[z].userSubscrito) == 0){
                                    printf("NOME: %s\n",tdC->cd[k].nome);
                                    int fd = open(tdC->cd[k].clientePipe, O_WRONLY);
                                    rsp.tipoResposta = 2;
                                    sprintf(resposta, "Mensagem nova: %s no topico: %s, por: %s",
                                        msg.topico.mensagem,msg.topico.topico, msg.clienteDados.nome);
                                    strcpy(rsp.msgRsp,resposta);
                                    printf("%s\n",rsp.msgRsp);
                                    write(fd,&rsp,sizeof(Resposta));
                                    close(fd);
                                }
                            }
                            //break;
                        }
                    }
                    escreveuMsg = 1;
                    break;
                }

            }
            pthread_mutex_unlock(tdC->mutex);
        }

        //TRATA DO COMANDO SUBSCRIBE DO CLIENTE
        else if (strcmp("subscribe",msg.tipoMSG)==0)
        {
            trataCriarSubscriber(tdC, &msg);
        }
        //TRATA DO COMANDO UNSUBSCRIBE DO CLIENTE
        else if (strcmp("unsubscribe",msg.tipoMSG)==0)
        {
            trataRemoverSubscriber(tdC,&msg);

        }
        //TRATA DO COMANDO EXIT DO CLIENTE
        else if (strcmp("exit",msg.tipoMSG) == 0)
        {
            printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
            rsp.tipoResposta = 99;
            write(pipeClienteResp,&rsp,sizeof(Resposta));
            close(pipe);
            unlink(serverPipes);
            break;

        }
    }
    pthread_exit(NULL);
}

void criaVariavelAmbiente() {
    if (putenv("MSG_FICH=../mensagens.txt") != 0) {
        printf("[ERRO] Erro ao criar variavel de ambiente.\n");
        exit(99);
    }
    //printf("MSG_FICH = %s\n", getenv("MSG_FICH"));
    system("export MSG_FICH");
}
void trataGuardarMensagens(ThreadData *td){
    int fd;
    char duracao[30];
    char *file = getenv("MSG_FICH");
    fd = open(file, O_WRONLY | O_CREAT, 0640);
    if (fd == -1)
        printf("[ERRO] ao abrir o ficheiro.");
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        if (strcmp(td->topicoTabela[i].topico,"-1") != 0 &&
            strcmp(td->topicoTabela[i].mensagem,"-1") != 0) {
            write(fd,td->topicoTabela[i].topico,strlen(td->topicoTabela[i].topico));
            write(fd," ",1);
            write(fd,td->topicoTabela[i].autor,strlen(td->topicoTabela[i].autor));
            write(fd," ",1);
            sprintf(duracao,"%d",td->topicoTabela[i].duracao);
            write(fd, duracao,strlen(duracao));
            write(fd," ",1);
            write(fd,td->topicoTabela[i].mensagem,strlen(td->topicoTabela[i].mensagem));
            write(fd,"\n",1);
        }
    }
    pthread_mutex_unlock(td->mutex);
    close(fd);
}
void trataLerMensagens(ThreadData *td) {
    FILE *fd;
    int duracao = 0;
    int escreveuMsg;
    char linha[430];
    char autor[50],topico[MAX_CARACTER_TOPICO],mensagem[MAX_CARACTER_MENSAGEM];
    int linhas = 1;
    char* file = getenv("MSG_FICH");
    fd = fopen(file,"r");
    if (fd == NULL) {
        printf("[ERRO] Erro ao abrir o ficheiro\n");
    }

    while (fgets(linha,430,fd) != NULL) {
        char *resultado = strtok(linha, " "); //para separar as palavras
        //estrair topico
        if(resultado != NULL) {
            strcpy(topico, resultado);
        }

        resultado = strtok(NULL, " ");
        if(resultado != NULL) {
            strcpy(autor, resultado);
        }
        resultado = strtok(NULL, " ");
        if(resultado != NULL) {
            duracao = atoi(resultado);
        }
        resultado = strtok(NULL, "");
        if(resultado != NULL) {
            strcpy(mensagem, resultado);
        }
        /*printf("-> %s %lu\n",topico,sizeof(topico));
        printf("-> %s %lu\n",autor,  sizeof(autor));
        printf("-> %d\n",duracao);
        printf("-> %s  %lu\n",mensagem,sizeof(mensagem));

        printf("%s",linha);
        printf("<- nLinha %d\n",linhas);*/
        linhas++;
        escreveuMsg=0;

        for(int i = 0; i < MAX_CARACTER_TOPICO; i++) {
            if(strcmp(td->topicoTabela[i].topico,"-1") == 0) {
                for (int j = i; j < i + 5; j++) {
                    strncpy(td->topicoTabela[j].topico,topico,strlen(topico));
                }
                strncpy(td->topicoTabela[i].mensagem,mensagem, strlen(mensagem)-1);
                strncpy(td->topicoTabela[i].autor,autor,strlen(autor));
                td->topicoTabela[i].duracao = duracao;
                td->topicoTabela[i].nMensagem++;
                escreveuMsg = 1;
                break;
            }
            if(strcmp(td->topicoTabela[i].topico,topico) == 0 && escreveuMsg == 0) {
                td->topicoTabela[i].nMensagem++;
                for (int j = i; j < i + 5; j++) {
                    if (strcmp(td->topicoTabela[j].mensagem,"-1") == 0) {
                        strncpy(td->topicoTabela[j].mensagem,mensagem, strlen(mensagem)-1);
                        strncpy(td->topicoTabela[j].autor,autor,strlen(autor));
                        td->topicoTabela[j].duracao = duracao;
                        break;
                    }
                }
                break;
            }
        }

    }
}
void trataRemoverSubscriber(ThreadData *td, Mensagem *msg) {
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        if (strcmp(td->sub[i].topico,msg->topico.topico) == 0 &&
            strcmp(td->sub[i].userSubscrito, msg->clienteDados.nome) == 0) { // Se existir na tabela subs
            strcpy(td->sub[i].userSubscrito,"-1");
            strcpy(td->sub[i].topico,"-1");
            break;
        }
    }
    pthread_mutex_unlock(td->mutex);

}
void trataCriarSubscriber(ThreadData* td,Mensagem* msg) {
    int subscribe = 0;
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        if (strcmp(td->topicoTabela[i].topico,msg->topico.topico) == 0) {
            for (int k = 0; k < MAX_LINHAS_TOPICOS; k++) {
                if (strcmp(td->sub[k].topico,msg->topico.topico) == 0 &&
                    strcmp(td->sub[k].userSubscrito, msg->clienteDados.nome) == 0) {
                    printf("O User: %s ja e subscriber do topico %s.\n",msg->clienteDados.nome,msg->topico.topico);
                    subscribe = 1;
                    break;
                }
            }
            if (subscribe == 0) {
                for (int j = 0; j < MAX_LINHAS_TOPICOS; j++) {
                    if (strcmp(td->sub[j].userSubscrito,"-1") == 0) {
                        strcpy(td->sub[j].userSubscrito,msg->clienteDados.nome);
                        strcpy(td->sub[j].topico,msg->topico.topico);
                        break;
                    }
                }
                break;
            }else {
                break;
            }
        }
    }
    pthread_mutex_unlock(td->mutex);
}

void respostaTopicos(ThreadData *td, int pipeClienteResp){
     Resposta rsp;
     rsp.tipoResposta = 0;
     pthread_mutex_lock(td->mutex);
     printf("%d\n",pipeClienteResp);
     for (int i = 0; i < MAX_LINHAS_TOPICOS; i++)
     {
         strcpy(rsp.topicoTabela[i].topico,td->topicoTabela[i].topico);
         rsp.topicoTabela[i].nMensagem = td->topicoTabela[i].nMensagem;
         strcpy(rsp.topicoTabela[i].mensagem,td->topicoTabela[i].mensagem);
         rsp.topicoTabela[i].duracao = td->topicoTabela[i].duracao;
     }
     write(pipeClienteResp,&rsp,sizeof(Resposta));
     pthread_mutex_unlock(td->mutex);

}
void mostraClientes(ThreadData *td){
    printf("\t\t+------------------+-+------------+-+----------------------+\n");
    printf("\t\t| Nome             | | Process ID | | Pipe                 |\n");
    printf("\t\t+------------------+-+------------+-+----------------------+\n");
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (strcmp(td->cd[i].nome, "-1") != 0){
            printf("\t\t| %-16s | | %-10d | | %-20s |\n" ,td->cd[i].nome, td->cd[i].PID, td->cd[i].clientePipe);
            printf("\t\t+------------------+-+------------+-+----------------------+\n");
        }
    }
    pthread_mutex_unlock(td->mutex);
}
void mostraTopicos(ThreadData *td){

    printf("+------------------------------+\n");
    printf("| TOPICO               | N_MSG |\n");
    printf("+------------------------------+\n");
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++)
    {
        if (strcmp(td->topicoTabela[i].topico,"-1") != 0 && strcmp(td->topicoTabela[i].mensagem,"-1") != 0) {
            if (i >= 0 && strcmp(td->topicoTabela[i].topico,td->topicoTabela[i-1].topico) != 0) {
                printf("| %-20s |", td->topicoTabela[i].topico);
            }else {
                printf("| %-20s |", "");
            }
            if (i >= 0 && td->topicoTabela[i].nMensagem != 0) {
                printf(" %-5d |\n", td->topicoTabela[i].nMensagem);
            }else {
                printf(" %-5s |\n", "");
            }
            printf("+------------------------------+\n");
        }
    }
    pthread_mutex_unlock(td->mutex);
}
void mostraMensagens(ThreadData *td,const char* topics) {
    printf("+---------------------------------------------------------------------------"
               "------------------------------------------------+\n");
    printf("| TOPICO               | MENSAGEM \t\t\t\t\t\t\t\t\t\t\t    |\n");
    printf("+---------------------------------------------------------------------------"
           "------------------------------------------------+\n");
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++)
    {
        if (strcmp(td->topicoTabela[i].topico,topics) == 0 &&
            strcmp(td->topicoTabela[i].mensagem,"-1") != 0) {
            if (i >= 0 && strcmp(td->topicoTabela[i].topico,td->topicoTabela[i-1].topico) != 0) {
                printf("| %-20s |", td->topicoTabela[i].topico);
            }else {
                printf("| %-20s |", "");
            }
            printf(" %-98s |\n" ,td->topicoTabela[i].mensagem);
            printf("+---------------------------------------------------------------------------"
                           "------------------------------------------------+\n");
        }
    }
    pthread_mutex_unlock(td->mutex);
}
void mostraEstados(ThreadData *td) {
    char estado[20];
    printf("+--------------------------------------+\n");
    printf("| TOPICO               | Estado        |\n");
    printf("+--------------------------------------+\n");
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++)
    {
        if (strcmp(td->topicoTabela[i].topico,"-1") != 0 && strcmp(td->topicoTabela[i].mensagem,"-1") != 0) {
            if (i >= 0 && strcmp(td->topicoTabela[i].topico,td->topicoTabela[i-1].topico) != 0) {
                if (td->topicoTabela[i].estados == 0)
                    strcpy(estado, "desbloqueado");
                else
                    strcpy(estado, "bloqueado");
                printf("| %-20s | %-13s |\n", td->topicoTabela[i].topico,
                    estado);
            }else {
                printf("| %-20s | %-13s |\n", "","");
            }
            printf("+--------------------------------------+\n");
        }
    }
    pthread_mutex_unlock(td->mutex);
}
void mostraSubscribes(ThreadData *td) {

    printf("+-----------------------------------+\n");
    printf("| TOPICO               | SUBSCRIBER |\n");
    printf("+-----------------------------------+\n");
    pthread_mutex_lock(td->mutex);
    for (int i = 0 ; i < MAX_LINHAS_TOPICOS ; i++) {
        if (strcmp(td->sub[i].topico,"-1") != 0) {
            printf("| %-20s | %-10s |\n", td->sub[i].topico,td->sub[i].userSubscrito);
            printf("+-----------------------------------+\n");
        }

    }
    pthread_mutex_unlock(td->mutex);
}
void inicializaTabelaClientes(ThreadData *td){

    for (int i = 0; i < MAX_USERS; i++)
    {
        td->cd[i].PID = 0;
        strcpy(td->cd[i].nome,"-1");
        strcpy(td->cd[i].clientePipe,"-1");
    }
}
void inicializaTabelaTopicos(ThreadData *td){

    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++)
    {
        strcpy(td->topicoTabela[i].topico,"-1");
        td->topicoTabela[i].nMensagem = 0;
        strcpy(td->topicoTabela[i].mensagem,"-1");
        td->topicoTabela[i].duracao = 0;
        td->topicoTabela[i].estados = 0;
        strcpy(td->topicoTabela[i].autor,"-1");

    }
}
void inicializaTabelaSubscricoes(ThreadData *td) {
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        strcpy(td->sub[i].topico,"-1");
        strcpy(td->sub[i].userSubscrito,"-1");
    }
}
void bloqueiaTopicos(ThreadData *td, const char* topics) {
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        if (strcmp(td->topicoTabela[i].topico,topics) == 0 &&
        strcmp(td->topicoTabela[i].mensagem,"-1") != 0) {
            if (i >= 0 && strcmp(td->topicoTabela[i].topico,td->topicoTabela[i-1].topico) != 0) {
                td->topicoTabela[i].estados = 1;
            }
        }
    }
    pthread_mutex_unlock(td->mutex);
}
void desbloqueiaTopicos(ThreadData *td, const char* topics) {
    pthread_mutex_lock(td->mutex);
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        if (strcmp(td->topicoTabela[i].topico,topics) == 0 &&
                    strcmp(td->topicoTabela[i].mensagem,"-1") != 0) {
            if (i >= 0 && strcmp(td->topicoTabela[i].topico,td->topicoTabela[i-1].topico) != 0) {
                td->topicoTabela[i].estados = 0;
            }
                    }
    }
    pthread_mutex_unlock(td->mutex);
}

void Menu(){


    printf("\t\e[0;32m+-------------------------------------------------------------------------------+\e[0m\n");
    printf("\t\e[0;32m|                              MANAGER - MENU                                   |\e[0m\n");
    printf("\t\e[0;32m+-------------------------------------------------------------------------------+\e[0m\n");
    printf("\t\e[0;32m| \e[1;34musers             \e[1;32m- Lista utilizadores ligados.                               \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mremove <username> \e[1;32m- Remove utilizador da plataforma.                          \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mtopics            \e[1;32m- Apresenta os topicos existentes na plataforma.            \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mshow <topico>     \e[1;32m- Apresenta todas as mensagens de um topico.                \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mlock <topico>     \e[1;32m- Bloqueia novas mensagens para para o topico selecionado.  \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34munlock <topico>   \e[1;32m- Desbloqueia topico.                                       \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mstatus            \e[1;32m- Mostra o estado dos topicos.                              \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34msubs              \e[1;32m- Mostra as subscricoes dos utilizadores.                   \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m| \e[1;34mclose             \e[1;32m- Encerra a plataforma.                                     \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m+-------------------------------------------------------------------------------+\e[0m\n");
    printf("#> ");

}