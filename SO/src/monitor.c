#include "utils.h"
#include <glib.h>
#include <stdbool.h>

ENTRY* entry_clone(ENTRY *e){
	ENTRY *novo = (ENTRY*) malloc(sizeof(struct Entry));
	novo->pid = e->pid;
	strcpy(novo->cmdName, e->cmdName);
	novo->timestamp = e->timestamp;
	return novo;
}

void add_element_to_new_table(gpointer key, gpointer value, gpointer new_table) {
	ENTRY *clone = entry_clone((ENTRY*)value);
    	g_hash_table_insert((GHashTable *)new_table, key, clone);
}

void sendStatus(GHashTable *new_table, pid_t pid){
	char s_pid[10];
	sprintf(s_pid, "%d", pid);

	if(mkfifo(s_pid, 0777) == -1)
		if(errno != EEXIST)
			perror("Could not create status fifo");

	int fd = open(s_pid, O_WRONLY);

	struct timeval a;
	gettimeofday(&a, NULL);
	GHashTableIter iter;
    	gpointer key, value;
    
	int size = g_hash_table_size(new_table);

	write(fd, &size, sizeof(int));

	g_hash_table_iter_init(&iter, new_table);
    	
	while (g_hash_table_iter_next(&iter, &key, &value)) {
        	ENTRY *v = (ENTRY*)value;
		ENTRY e;
		e.pid = v->pid;
		strcpy(e.cmdName, v->cmdName);
		e.timestamp.tv_sec = a.tv_sec - v->timestamp.tv_sec;
		e.timestamp.tv_usec = a.tv_usec - v->timestamp.tv_usec;
		write(fd, &e, sizeof(ENTRY));
    	}

	g_hash_table_destroy(new_table);

	close(fd);
}

//escreve a info para os ficheiros de cada pid
void fWriter(int pipe,char *path){
	ENTRY e;
	int res;
	int fd=-1;
	char string[64];

	mkdir(path,0777);
	
	while((res = read(pipe, &e, sizeof(e))) > 0){
		
		snprintf(string,64,"%s/%d",path,e.pid);
		
		fd = open(string, O_CREAT | O_APPEND | O_WRONLY, 0600);
		if(fd == -1)
			perror("Failed to open file exec stats!\n");
		
		write(fd, &e, res);
	}

	close(fd);
	close(pipe);
	_exit(0);
}

ENTRY resetENTRY(){
	ENTRY new;
	return new;
}


void sendStatsTime(pid_t pid,char * path){
	
	char s_pid[10];
	sprintf(s_pid, "%d", pid);
	int res;
	int aux;
	long int duration=-1;
	char string[64];

	ENTRY e;

	if(mkfifo(s_pid, 0777) == -1)
		if(errno != EEXIST)
			perror("Could not create status fifo");

	int fd = open(s_pid, O_RDONLY);
	int fd2;

	while((res = read(fd, &aux, sizeof(int))) > 0){

		snprintf(string,64,"%s/%d",path,aux);
		
		fd2 = open(string,O_RDONLY,0600);
		
		if(fd2 == -1){
			printf("Invalid pid: %d\n",aux);
		}
			
		else{
			read(fd2,&e,sizeof(e));	

			duration += (e.timestamp.tv_sec*1000) + (e.timestamp.tv_usec/1000);
			close(fd2);
		}
	}

	close(fd);

	fd = open(s_pid,O_WRONLY);

	write(fd,&duration,sizeof(long int));

	close(fd);
	_exit(0);

}


void sendStatsCommand(pid_t pid,char *path){
	char s_pid[10];
	sprintf(s_pid, "%d", pid);
	int res;
	int aux;
	int usage=0;
	char string[64];
	char comando[NAMESIZE];
	char **comandos;
	ENTRY e;

	if(mkfifo(s_pid, 0777) == -1)
		if(errno != EEXIST)
			perror("Could not create status fifo");

	int fd = open(s_pid, O_RDONLY);
	int fd2;
	
	read(fd,&comando,NAMESIZE);
	
	while((res = read(fd, &aux, sizeof(int))) > 0){
		snprintf(string,64,"%s/%d",path,aux);
		
		fd2 = open(string,O_RDONLY,0600);
		
		if(fd2 == -1){
			printf("Invalid pid: %d\n",aux);
		}
			
		else{
			read(fd2,&e,sizeof(e));	

			comandos = parsePipes(e.cmdName);

			for(int i=0;comandos[i]!=NULL;i++){
				if(!strcmp(e.cmdName,comandos[i])) usage ++;
			}
			close(fd2);
		}
	}

	close(fd);

	fd = open(s_pid,O_WRONLY);

	write(fd,&usage,sizeof(int));

	close(fd);
	_exit(0);
}

void sendStatsUniq(pid_t pid,char * path){
	char s_pid[10];
	sprintf(s_pid, "%d", pid);
	int res;
	int aux;
	char string[64];
	ENTRY e;
	char *strAux;
	char **comandos;
	GHashTable *programas = g_hash_table_new(g_str_hash, g_str_equal); //str hash (id é uma string), str_equal (compara strings)

	if(mkfifo(s_pid, 0777) == -1)
		if(errno != EEXIST)
			perror("Could not create status fifo");

	int fd = open(s_pid, O_RDONLY);
	int fd2;

	while((res = read(fd, &aux, sizeof(int))) > 0){
		snprintf(string,64,"%s/%d",path,aux);
		
		fd2 = open(string,O_RDONLY,0600);
		
		if(fd2 == -1){
			printf("Invalid pid: %d\n",aux);
		}
			
		else{
			read(fd2,&e,sizeof(e));	
			
			comandos = parsePipes(e.cmdName);

			for(int i=0;comandos[i]!=NULL;i++){

				if (!g_hash_table_contains(programas,comandos[i])){
					strAux = strdup(comandos[i]);
    				g_hash_table_insert(programas,strAux, NULL);
				}

			}
			
			
		
			close(fd2);
		}
	}

	close(fd);

	fd = open(s_pid,O_WRONLY);
	

	GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, programas);//funcao que itera sobre todas as linhas da hash table
    
    while (g_hash_table_iter_next(&iter, &key, &value)){
		write(fd,key,NAMESIZE);
    }

	close(fd);
	_exit(0);
}

int main(int argc, char** argv){

	if(mkfifo("stats", 0777) == -1)
		if(errno != EEXIST){
			perror("Could not create fifo file");
		}

	int fd = open("stats", O_RDONLY);
	if(fd == -1){
		perror("Failed to open FIFO\n");
	}
	int fd2 = open("stats", O_WRONLY);
	if(fd2 == -1){
		perror("Failed to open FIFO\n");
	}

	int child_pipe[2];

	if(pipe(child_pipe) == -1){
		perror("Failed to create pipe to file writer!\n");
	}	
	
	if(argc==1){//se nao se passar o nome da pasta fica default
		if(fork() == 0){
			close(child_pipe[1]);
			fWriter(child_pipe[0],"stats_files");
		}
	}

	else if(argc==2){
		if(fork() == 0){
		close(child_pipe[1]);
		fWriter(child_pipe[0],argv[1]);
		}
	}
	else{
		perror("Invalid command number!");
	}


	close(child_pipe[0]);

	
	GHashTable *process = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
	ENTRY e;

	int res;
	bool flag = true;
	bool abort = false;

	while(flag && ((res = read(fd, &e, sizeof(ENTRY))) > 0)){
		if(!strcmp(e.cmdName,"abort")){
			printf("aborting...\n");
			abort = true;
		}
		
		else if(!strcmp(e.cmdName, "status")){
			printf("[%d] Asked for Status\n", e.pid);
			GHashTable *new_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, free);
    			g_hash_table_foreach(process, add_element_to_new_table, new_table);

			if(fork() == 0)
				sendStatus(new_table, e.pid);
		}

		else if(!strcmp(e.cmdName,"stats-time")){
			printf("[%d] Asked for stats-time\n",e.pid);
			
			if(fork() == 0){
				if(argc==1){
					sendStatsTime(e.pid,"stats_files"); //para o caso de ser o path default para os ficheiros
				}
				else if(argc==2){
					sendStatsTime(e.pid,argv[1]); //para o caso de ser o path passado por argumento
				}	
			}
		}

		else if(!strcmp(e.cmdName,"stats-command")){
			printf("[%d] Asked for stats-command\n",e.pid);
			
			if(fork()==0){
				if(argc==1){
					sendStatsCommand(e.pid,"stats_files"); //para o caso de ser o path default para os ficheiros
				}
				
				else if(argc==2){
					sendStatsCommand(e.pid,argv[1]); //para o caso de ser o path passado por argumento
				}
			}
		}
		else if(!strcmp(e.cmdName,"stats-uniq")){
			printf("[%d] Asked for stats-uniq\n",e.pid);

			if(fork()==0){
			
				if(argc==1){
					sendStatsUniq(e.pid,"stats_files"); //para o caso de ser o path default para os ficheiros
				}
				else if(argc==2){
					sendStatsUniq(e.pid,argv[1]); //para o caso de ser o path passado por argumento
				}
			}
		}
	

		else if(g_hash_table_contains(process, GINT_TO_POINTER((int)e.pid)) == TRUE){
            printf("[%d] Finished Command %s\n", e.pid, e.cmdName);
            ENTRY *v = g_hash_table_lookup(process, GINT_TO_POINTER((int)e.pid));

            //fica já no ficheiro guardado com o tempo de duração do comando
            e.timestamp.tv_sec = e.timestamp.tv_sec - v->timestamp.tv_sec;
            e.timestamp.tv_usec = e.timestamp.tv_usec - v->timestamp.tv_usec;
            
        
            write(child_pipe[1], &e, sizeof(e));
            g_hash_table_remove(process, GINT_TO_POINTER((int)e.pid));
		} else {

			ENTRY *clone = entry_clone(&e);

			printf("[%d] Started Executing %s\n", e.pid, e.cmdName);
			g_hash_table_insert(process, GINT_TO_POINTER((int)e.pid), clone);
		}
		e = resetENTRY();
		if(g_hash_table_size(process) == 0 && abort == true){
			flag = false;
		}
	}

	close(child_pipe[1]);
	close(fd);

	g_hash_table_destroy(process);

	return 0;
}
