/*
 * project 1 (shell) main.c template 
 *
 * project partner names and other information up here, please
 *
 */

/* you probably won't need any other header files for this project */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
//Global variables used in functions
char m = 's', mode_ret;


struct rusage timeusage, Ctimeusage;
struct timeval usertime,childusertime, kerneltime,childkerneltime;
long double totusertime = 0.0, totkerneltime = 0.0;

void time(void){
	getrusage(RUSAGE_SELF,&timeusage); getrusage(RUSAGE_CHILDREN,&Ctimeusage);
	usertime = timeusage.ru_utime; childusertime=Ctimeusage.ru_utime;
	totusertime=usertime.tv_sec+childusertime.tv_sec+(usertime.tv_usec+childusertime.tv_usec)*(.000001);
	kerneltime=timeusage.ru_stime; childkerneltime=Ctimeusage.ru_stime;
	totkerneltime=kerneltime.tv_sec+childkerneltime.tv_sec+(kerneltime.tv_usec+childkerneltime.tv_usec)*(.000001);
	printf("Total user time: %08Lf, Total kernel time: %08Lf\n",totusertime,totkerneltime);
	return;
}


char mode(char** cmd, int numstrings, char m){
	if(numstrings < 2){
		if(cmd[0][0]!='m'){
			return 'f';
		}
		if(m == 's'){
			printf("sequential mode\n");
			return 's';
		}else if(m == 'p'){
			printf("parallel mode\n");
			return 'p';
		}else{
			return 'f';
		}
	}else if(!strcasecmp(cmd[1], "sequential")||cmd[1][0] == 's'||!strcasecmp(cmd[1], "seq")){
		if(m=='s'){
			printf("You are already in sequential mode.\n");
		}else{
			printf("Going into sequential mode after jobs finish.\n");
		}
		return 's';
	}else if(!strcasecmp(cmd[1], "parallel")||cmd[1][0] == 'p'||strcasecmp(cmd[1], "par")){
		if(m=='p'){
			printf("You are already in parallel mode.\n");
		}else{
			printf("Going into parallel mode.\n");
		}
		return 'p';
	}
	return 'f';
}


void loopfree(char** arr, int numcommands){
	int i = 0;
	if(numcommands ==0){
		for(;arr[i]!=NULL;i++){
			free(arr[i]);
		}
	}else{
		for(;i<numcommands;i++){
			free(arr[i]);
		}
	}
	free(arr);
}

char* checkpath(char* command, char** prepaths, int numpaths){
	int test = 0, i = 0;
	char cmd[1024];
	char path[1024];
	char* c = strdup(command);
	struct stat statinfo;
	printf("orig cmd: %s\n",command);
	if(prepaths == NULL || (test=stat(command,&statinfo))==0){
		return c;
	}
	char* cmdpath;
	for(;i<numpaths;i++){
		strcpy(cmd,command); strcpy(path,prepaths[i]);
		cmdpath=malloc(sizeof(char)*(strlen(cmd)+strlen(path))+1);
		sprintf(cmdpath,"%s%s",path, cmd);
		if((test = stat(cmdpath,&statinfo))==0){
			printf("path prepended: %s\n",cmdpath);
			free(c);
			return cmdpath;
		}
		free(cmdpath);
	}
	return c;
}

int Numstrings(char* string, char delim){
	int i = 0, count = 1;
	for(;i<strlen(string);i++){
		if(string[i] == delim){
			count++;
		}
	}
	return count;
}

void removeextrainput(char* s){
	if(s==NULL){
		return;}
	int len = strlen(s), i = 0, j = 0, numspaces = 0, space = 0;
	int text = 0, symbol = 0, remove = 0, numcolons = 0;
	for(; i < len; i++){
		if(s[i] == '#'){
			s[i] = '\0';break;
		}
	}
	i = 0; len =strlen(s);
	while(i<len-numcolons){
		if(s[i]==';'){
			j=i+1; symbol++;
			if(text==0){
				remove = 1;
			}
		}else if(isalpha(s[i]) ||(!isspace(s[i]) && s[i]!=';')){
			text = 1; symbol = 0; remove = 0;
		}
		if(symbol > 1 || remove > 0){
			while(j<len-numcolons){
				s[j-1] = s[j]; j++;
			}
			
			symbol--; numcolons++; remove = 0;
			continue;
		}
		i++;
	}
	s[len-numcolons]='\0';
	i=0,j=0,len = strlen(s), text = 0, symbol = 0;
	while(i<len-numspaces){
		if(isspace(s[i])){
			j = i +1; space++; //know its a space but not sure if its 2 spaces in a row.
			if(text == 0 || symbol == 1){
				space++;
			}
			if(s[i] == '\n' || s[i] == '\t' || s[i] == '\r'){
				space++; //makes it so it definitely is removed
			}
		}else if(s[i] == ';'){
			symbol = 1;
			if(space>0){
				space++;
			}
		}
		if(space >1){
			while(j<len -numspaces){
				s[j-1] = s[j]; j++;
			}
			space--; numspaces++;
			if(symbol == 1){
				space--;
			}
			continue;
		}
		if(isalpha(s[i]) || (!isspace(s[i])&&s[i]!=';')){
			space = 0;
			text = 1; symbol = 0;
		}
		i++;
	}
	if(isspace(s[i-1])||s[i-1]==';'){
		s[i-1] = '\0';
	}
	else{
		s[len-numspaces] = '\0';
	}
}

char** tokenify(char *s, char* delim){
	if(s==NULL){
		return NULL;}
	int len = strlen(s);
	char** tokstrings;
	int i = 0, numtokens = 0;
	for(;i<len;i++){
		if(s[i] == delim[0]){
			numtokens++;
		}
	}
	if(numtokens==0){
		tokstrings = malloc(sizeof(char*) +sizeof(NULL));
		tokstrings[0] = strdup(s); tokstrings[1] = NULL;
		return tokstrings;}
	tokstrings = malloc(sizeof(char *) * (numtokens + 1)+sizeof(NULL));
	tokstrings[0] = strdup(strtok(s,delim)); i = 1;
	while(i<numtokens+1){
		tokstrings[i] = strdup(strtok(NULL, delim));
		i++;
	}
	tokstrings[i] = NULL;
	return tokstrings;
}		

int numpaths(){
	FILE* file = fopen("shell-config","r");
	char buffer[1024]; int count = 0;
	while(fgets(buffer,1024,file)!=NULL){
		count++;
	}
	fclose(file);
	return count;
}
		
char** patharray(int npaths){
	int i = 0;
	char path[1024];
	char** paths = malloc(sizeof(char*)*npaths+1);
	FILE* file = fopen("shell-config", "r");
	if(file == NULL){
		fclose(file);
		printf("Missing PATH file. You will have to enter the full command name to execute.\n");
		return NULL;
	}
	while(fgets(path,1024,file)!=NULL){
		if(path[strlen(path)-1] == '\n'){
			path[strlen(path)-1] = '\0';
		}
		paths[i] = strdup(path); i++;
	}
	paths[i] = NULL;
	fclose(file);
	return paths;
}

 struct process {
        int pid[10];
		char *status;
		char remove = 'n';
		char** command;
        struct processes *next; 
		struct processes *prev;
    };
    
void list_insert(pid_t* childpids, int numpids, char*** pcommands,struct process **head) {
	struct process *temp = *head; int i = 0;
	if(*head == NULL){
		struct process *newnode = malloc(sizeof(struct node));
		newnode->pid = childpids[i]; newnode->status = "running"; newnode->command = pcommands[i];i++;
		*head = newnode;
	}
	for(;i<numpids;i++){
		struct process *newnode = malloc(sizeof(struct node));
		newnode->pid = childpids[i]; newnode->status = "running";newnode->command = pcommands[i];
		while(temp->next !=NULL){              
			temp = temp->next;
		}
		temp->next = newnode; newnode->prev = temp;
	}
}

void process_remove(struct processes **head){
	struct process *temp = *head, *prev;
	while(temp != NULL){
		if(temp->remove == 'y'){
			if(temp->prev == NULL){
				printf("Process: %d (%s) completed.\n",temp->pid,temp->command);
				free(*head->command);free(**head); *head = temp->next; *head->prev = NULL; temp = *head; continue;
			}else{
				prev->next = temp->next; temp = temp->next; free(temp->prev);temp->prev = prev;
			}
		}
		prev = temp; temp = temp->next;
	}
		 

int parallel(char** commands, int numcommands, int i/*commandnum*/, char** prepaths){
	struct process *head = NULL;
	struct pollfd input[1];
	input[0].fd = 0; input[0].events = POLLIN;
	char buffer[1024]; *prompt = "INPUT$: ";
	char*** = pcommands;
	int pollstatus = 0, numchildren = numcommands - i, k = 0,j=0,t=0,numstrings=0,flag = 0,npaths=numpaths(),nulltest = 0;
while(1){
	pollstatus = poll(input,1,750);
	if(pollstatus == 0){
		pcommands = malloc(sizeof(char**)*numcommands);
		pid_t* childPIDS;
		char** cmd;
		childPIDS = malloc(sizeof(pid_t)*numchildren+sizeof(NULL));
		pid_t childpid;
	for(;i<numcommands;i++){
		numstrings = Numstrings(commands[i], ' ');
		char** tcmd = tokenify(commands[i], " ");
		char* tempcmd = checkpath(tcmd[0],prepaths,npaths);
		char** cmd = malloc(sizeof(char*)*numstrings+sizeof(NULL)); int t = 1;
		cmd[0] = tempcmd;
		for(;t<numstrings;t++){
			cmd[t] = strdup(tcmd[t]);
		}
		cmd[t] = NULL;		
		if((!strcasecmp(cmd[0], "mode"))||(strlen(cmd[0])==1)){
			mode_ret = mode(cmd, numstrings, m);
			if(mode_ret == m){
			loopfree(cmd, numstrings); loopfree(tcmd, numstrings); numchildren--; 
			continue;
			}else if(mode_ret == 'f'){
			}else{
				m = mode_ret; loopfree(cmd, numstrings);loopfree(tcmd,numstrings); numchildren--;
				continue;
			}
		}
		if(!strcasecmp(cmd[0], "exit")){
			flag = 1; numchildren--;
			loopfree(cmd, numstrings); loopfree(tcmd, numstrings);printf("Will exit shell once all jobs have finished.\n");
		 	continue;
		}
		if((childpid=fork())==0){
			printf("In child: %d with PID: %d\n", j, getpid());
							
			if (execv(cmd[0], cmd) < 0) {
		 		fprintf(stderr, "execv failed: %s\n", strerror	(errno)); 
				loopfree(cmd,numstrings); loopfree(commands,numcommands); free(childPIDS);
				loopfree(prepaths,npaths);loopfree(tcmd,numstrings);
				exit(0);
  			}

		}else{
			childPIDS[j] = childpid;
		}
		pcommands[j] = cmd;
		loopfree(tcmd,numstrings);
		j++;
	}
	list_insert(childPIDS,numchildren,pcommands,&head);
	free(childPIDS);
	struct process* temp = head;
	while(temp != NULL){
		int rstatus=0;
		pid_t child;
		if(temp->pid>0){
			child = waitpid(temp->pid,&rstatus,WNOHANG);
			if(child>0){
				printf("Parent got carcass of child process %d, return val %d\n", child, rstatus);
				assert(temp->pid == child);
				temp->pid = 0; temp->remove = 'y'; temp->status="done";
			}
		}
		temp = temp->next;
	}
	process_remove(&head);	
	if(flag > 0){
		loopfree(commands, numcommands); loopfree(prepaths,npaths);time();
		exit(0);
	}
	i = 0, numcommands = 0, k=0,j=0,t=0,flag=0; 
	free(pcommands);
	}else if(status >0){
		nulltest = fgets(buffer, 1024, stdin);
		if(nulltest == NULL){
			return 1;
		}
		buffer[strlen(buffer)-1]='\0';
		removeextrainput(buffer);
		numcommands = Numstrings(buffer, ';'); commands = tokenify(buffer, ";");
		printf("%s",prompt); fflush(stdout);
	}
}

int 
main(int argc, char **argv) {
	int i = 0, numcommands=0,numstrings=0, npaths = numpaths(), parallel_ret=0;
    char *prompt = "INPUT$: ", nulltest;
	char **commands;
	char **prepaths = patharray(npaths);
    printf("%s", prompt);
    fflush(stdout);
    char buffer[1024];
    while ((nulltest=fgets(buffer, 1024, stdin)) != NULL) {
        /* process current command line in buffer */
		i = 0;
		int len = strlen(buffer);
		buffer[len -1] = '\0';
		removeextrainput(buffer);
		numcommands = Numstrings(buffer, ';');
		commands = tokenify(buffer, ";");
		if(commands == NULL){
			printf("Error reading input.");
			printf("%s", prompt);
			fflush(stdout);
			continue;
		}
		while(i < numcommands){
			if(m=='p'){
				parallel_ret = parallel(commands, numcommands, i,prepaths);
				//if(parallel_ret == 1){
				//	exit(0);          
				break;
			}
			numstrings = Numstrings(commands[i], ' ');
			char** tcmd = tokenify(commands[i], " ");
			char* tempcmd = checkpath(tcmd[0],prepaths,npaths);
			printf("tempcmd: %s\n",tempcmd);
			char** cmd = malloc(sizeof(char*)*numstrings+sizeof(NULL)); int t = 1;
			cmd[0] = tempcmd;
			printf("cmd[0]: %s\n",cmd[0]);
			for(;t<numstrings;t++){
				cmd[t] = strdup(tcmd[t]);
				printf("%d\n",t);
			}
			cmd[t] = NULL;
			if(!strcasecmp(cmd[0], "exit")){
				loopfree(commands,numcommands);
				loopfree(cmd, numstrings);loopfree(prepaths,npaths);loopfree(tcmd, numstrings);time();
				exit(0);
			}
			if((!strcasecmp(cmd[0], "mode"))||(strlen(cmd[0])==1)){
					mode_ret = mode(cmd, numstrings, m);
					if(mode_ret == m){
						i++; loopfree(cmd, numstrings);loopfree(tcmd,numstrings); continue;
					}else if(mode_ret == 'f'){
					}else{
						m = mode_ret; loopfree(cmd, numstrings);loopfree(tcmd,numstrings); i++; continue;
					}
			}
       		pid_t p = fork();
        	if (p == 0) {
            /* in child */
				
            	if (execv(cmd[0], cmd) < 0) {
                	fprintf(stderr, "execv failed: %s\n", strerror	(errno)); 
					loopfree(cmd,numstrings);loopfree(commands,numcommands);
					loopfree(prepaths,npaths);loopfree(tcmd,numstrings);
					exit(0);
            	}
			} else if (p > 0) {
            /* in parent */
            	int rstatus = 0;
            	pid_t childp = wait(&rstatus);

            /* for this simple starter code, the only child process we should
               "wait" for is the one we just spun off, so check that we got the
               same process id */ 
            	assert(p == childp);

            	printf("Parent got carcass of child process %d, return val %d\n", childp, rstatus);
        	} else {
            /* fork had an error; bail out */
            	fprintf(stderr, "fork failed: %s\n", strerror(errno));
        	}
			
			i++; //next command
			loopfree(cmd, numstrings); loopfree(tcmd,numstrings);
		}
		loopfree(commands, numcommands);
		//all commands have finished
        printf("%s", prompt);
        fflush(stdout);
    }
	if(nulltest==NULL){
		loopfree(prepaths,npaths);time();
		return 0;
	}
	loopfree(commands, numcommands); loopfree(prepaths,npaths);
	time();
    return 0;
}



	
