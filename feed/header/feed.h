#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

//estruturas...
typedef struct
{
    char* nome[100];
    int PID;
}ClienteDados;

//defines

#define SERVER_PIPE "../SERVER"

void Menu();