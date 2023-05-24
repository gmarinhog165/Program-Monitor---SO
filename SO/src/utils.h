#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#define NAMESIZE 70

typedef struct Entry {
	pid_t pid;
	char cmdName[NAMESIZE];
	struct timeval timestamp;
} ENTRY;


void removeEnters(char *string){//remove enters xDD
    int size = strlen(string);
    size--;

    if(string[size]=='\n'){
    string[size]='\0';
    }
}


char** parsePipes(char *cmd){
    char **array = malloc(20 * sizeof(char*));
    int i = 0;
    char *aux;

    while((aux = strsep(&cmd, "|")) != NULL){
        if(i!=0) array[i++] = aux+1;
        else array[i++] = aux;
    }

    array[i] = NULL;
    return array;
}