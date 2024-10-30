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
    char user[100], topics[MAX_CARACTER_TOPICO];
    ClienteDados cd;
    Resposta rsp;
    union sigval sv; //o mais certo é isto não ficar assim...

    fgets(comando, sizeof(comando), stdin);
    comando[strcspn(comando, "\n")] = 0;
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
    //COMANDO PARA TERMINAR O MANAGER E INFORMAR OS FEED'S
    else if(strcmp(comando,"close") == 0){
        td->continua = 0;
        rsp.tipoResposta = 99;
        int res;
        for(int i = 0; i < MAX_USERS; i++) {
            if (td->pipeCliente[i] != -1) { //só faz para os clientes ligados
                printf("PIPE DIZ %d",td->pipeCliente[i]);
                fflush(stdout);
                res = write(td->pipeCliente[i], &rsp.tipoResposta,sizeof(rsp.tipoResposta));
                printf("[res] %d\n",res);
            }
        }
        write(td->pipeServer, "", sizeof("")); //só para destrancar o pipe na thread
    }

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
    int nMsg = 0;
    int pipe = open(SERVER_PIPECLIENTE, O_RDONLY);
    int escreveuMsg;

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
        escreveuMsg = 0;
        read(pipe, &msg, sizeof(Mensagem)); //recebe as mensagens de comando
        printf("THREAD TRATACOMANDOS: %s\n", msg.tipoMSG);
        fflush(stdout);
        //TRATA DO COMANDO TOPICS DO CLIENTE
        if (strcmp("topics",msg.tipoMSG)==0)
        {
            respostaTopicos(tdC, pipeClienteResp);
        }
        //TRATA DO COMANDO MSG DO CLIENTE
        else if (strcmp("msg",msg.tipoMSG)==0) {
            printf("[RECEBI DO CLIENTE TIPO] %s\n",msg.tipoMSG);
            printf("[RECEBI DO CLIENTE NOMETOPIC] %s\n",msg.topico.topico);
            printf("[RECEBI DO CLIENTE DURACAO] %d\n",msg.topico.duracao);
            printf("[RECEBI DO CLIENTE MENSAGEM] %s\n",msg.topico.mensagem);


            for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
                // para a primeira vez...
                if (strcmp(tdC->topicoTabela[i].topico,"-1") == 0){
                    for (int j = i; j < i+5; j++) {
                        strcpy(tdC->topicoTabela[j].topico,msg.topico.topico);
                    }
                    if (strcmp(tdC->topicoTabela[i].topico,msg.topico.topico) == 0)
                        tdC->topicoTabela[i].nMensagem++; //incremento nMensages
                    if (strcmp(tdC->topicoTabela[i].topico,msg.topico.topico) == 0) {
                        strcpy(tdC->topicoTabela[i].mensagem,msg.topico.mensagem);
                        tdC->topicoTabela[i].duracao = msg.topico.duracao;
                        escreveuMsg = 1;
                        break;
                    }
                }
                if (strcmp(tdC->topicoTabela[i].topico,msg.topico.topico) == 0 && escreveuMsg == 0) {
                    printf("CHEGUEI AQUI!\n");
                    fflush(stdout);
                    tdC->topicoTabela[i].nMensagem++; //ver isto
                    for (int j = i; j < i + 5; j++) {
                        if (strcmp(tdC->topicoTabela[j].mensagem,"-1") == 0) {
                            strcpy(tdC->topicoTabela[j].mensagem,msg.topico.mensagem);
                            tdC->topicoTabela[j].duracao = msg.topico.duracao;
                            break;
                        }
                    }
                    //strcpy(tdC->topicoTabela[i+1].mensagem,msg.topico.mensagem);
                    tdC->topicoTabela[i+1].duracao = msg.topico.duracao;
                    escreveuMsg = 1;
                    break;
                }

            }
        }

        //TRATA DO COMANDO SUBSCRIBE DO CLIENTE
        else if (strcmp("subscribe",msg.tipoMSG)==0)
        {
            printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
        }
        //TRATA DO COMANDO UNSUBSCRIBE DO CLIENTE
        else if (strcmp("unsubscribe",msg.tipoMSG)==0)
        {
            printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
        }
        //TRATA DO COMANDO EXIT DO CLIENTE
        else if (strcmp("exit",msg.tipoMSG)==0)
        {
            printf("[RECEBI DO CLIENTE] %s\n",msg.tipoMSG);
            //tenho de retirar o cliente depois aqui se não fica em loop
            // e informar os outros utilizadores
        }

    }
    
}
void respostaTopicos(ThreadData *td, int pipeClienteResp){
    Resposta rsp;
    rsp.tipoResposta = 0;
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++)
    {
        strcpy(rsp.topicoTabela[i].topico,td->topicoTabela[i].topico);
        rsp.topicoTabela[i].nMensagem = td->topicoTabela[i].nMensagem;
        strcpy(rsp.topicoTabela[i].mensagem,td->topicoTabela[i].mensagem);
        rsp.topicoTabela[i].duracao = td->topicoTabela[i].duracao;
        /*printf("|%s|%d|%s|%d|",rsp.topicos[i].topico,
            rsp.topicos[i].nMensagem,
            rsp.topicos[i].mensagem,
            rsp.topicos[i].duracao);
        fflush(stdout);*/
    }
    
    /*int res = */write(pipeClienteResp,&rsp,sizeof(Resposta));
}
void mostraClientes(ThreadData *td){
    printf("\t\t+------------------+-+------------+-+----------------------+\n");
    printf("\t\t| Nome             | | Process ID | | Pipe                 |\n");
    printf("\t\t+------------------+-+------------+-+----------------------+\n");
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (strcmp(td->cd[i].nome, "-1") != 0){
            printf("\t\t| %-16s | | %-10d | | %-20s |\n" ,td->cd[i].nome, td->cd[i].PID, td->cd[i].clientePipe);
            printf("\t\t+------------------+-+------------+-+----------------------+\n");
        }
    }
}
void mostraTopicos(ThreadData *td){

    printf("+------------------------------+\n");
    printf("| TOPICO               | N_MSG |\n");
    printf("+------------------------------+\n");
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
}
void mostraMensagens(ThreadData *td,const char* topics) {
    printf("+---------------------------------------------------------------------------"
               "------------------------------------------------+\n");
    printf("| TOPICO               | MENSAGEM \t\t\t\t\t\t\t\t\t\t\t    |\n");
    printf("+---------------------------------------------------------------------------"
           "------------------------------------------------+\n");
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
}
void mostraEstados(ThreadData *td) {
    char estado[20];
    printf("+--------------------------------------+\n");
    printf("| TOPICO               | Estado        |\n");
    printf("+--------------------------------------+\n");
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

    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++)
    {
        strcpy(td->topicoTabela[i].topico,"-1");
        td->topicoTabela[i].nMensagem = 0;
        strcpy(td->topicoTabela[i].mensagem,"-1");
        td->topicoTabela[i].duracao = 0;
        td->topicoTabela[i].estados = 0;
    }
}
void bloqueiaTopicos(ThreadData *td, const char* topics) {
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        if (strcmp(td->topicoTabela[i].topico,topics) == 0 &&
                    strcmp(td->topicoTabela[i].mensagem,"-1") != 0) {
            if (i >= 0 && strcmp(td->topicoTabela[i].topico,td->topicoTabela[i-1].topico) != 0) {
                td->topicoTabela[i].estados = 1;
            }
                    }
    }
}
void desbloqueiaTopicos(ThreadData *td, const char* topics) {
    for (int i = 0; i < MAX_LINHAS_TOPICOS; i++) {
        if (strcmp(td->topicoTabela[i].topico,topics) == 0 &&
                    strcmp(td->topicoTabela[i].mensagem,"-1") != 0) {
            if (i >= 0 && strcmp(td->topicoTabela[i].topico,td->topicoTabela[i-1].topico) != 0) {
                td->topicoTabela[i].estados = 0;
            }
                    }
    }
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
    printf("\t\e[0;32m| \e[1;34mclose             \e[1;32m- Encerra a plataforma.                                     \e[0;32m|\e[0m\n");
    printf("\t\e[0;32m+-------------------------------------------------------------------------------+\e[0m\n");
    printf("#> ");

}