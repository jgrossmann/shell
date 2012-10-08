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

void Remove(char* string, int index){
	int i = index +1;
	for(;i<strlen(string)-1;i++){
		string[i-1] = string[i];
	}
	string[i] = NULL;
	return string;
}

void cleancolons(char* string){
	int i = 0, numremoved = 0, text = 0, symbol = 0;
	for(;i<strlen(string);i++){
		if(string[i] == ';'){
			if(text == 0){
				string = Remove(string, i); numremoved++; i--; continue;
			}else if(symbol >0){
				string = Remove(string, i); numremoved++; i--; continue;
			}else{
				symbol = 1;
			}
		}

		if(isalpha(string[i]) || (!isspace(string[i]) && string[i]!=';')){
			text = 1; symbol = 0;
		}
	}
	if(string[i-1] == ';'){
		string[i-1] = NULL;
	}
	return string;
}

char mode(char** cmd, int numstrings, char m){
	if(numstrings < 2){
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
			printf("Going into sequential mode\n");
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
}

void loopfree(char** arr, int numcommands){
	int i = 0;
	for(;i<numcommands;i++){
			free(arr[i]);
	}
	free(arr);
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

void removeextrawhitespace(char* s){
	if(s==NULL){
		return;}
	int len = strlen(s), i = 0, j = 0, numspaces = 0, space = 0;
	int text = 0, symbol = 0;
	while(i<len-1){
		if(isspace(s[i])){
			j = i +1; space++; //know its a space but not sure if its 2 spaces in a row.
			if(text == 0 || symbol == 1){
				space++;
			}
			if(s[i] == '\n' || s[i] == '\t' || s[i] == '\r'){
				if(space<2){
					space++; //makes it so it definitely is removed
				}
			}
		}else if(s[i] == ';'){
			symbol++;
			if(space>0){
				space++;
			}
		}
		if(isalpha(s[i]) || (!isspace(s[i])&&s[i]!=';')){
			if(symbol<1){
				space = 0; 
				text = 1;
			}else{
				symbol = 0; text = 1;
			}
		}
		if(space >1){
			while(j<len -numspaces){
				s[j-1] = s[j]; j++;
			}
			space--; numspaces++;
			continue;
		}
		
		i++;
	}
	if(isspace(s[i])){
		s[i] = NULL;
	}
	else{
		s[len-numspaces+1] = NULL;
	}
}

char** tokenify(char *s, char* delim){
	if(s==NULL){
		return NULL;}
	printf("tok%s\n",s);
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


char** useableinput(char* s){
	if(s==NULL){
		return NULL;
	}
	int len = strlen(s), i = 0;
	char** useablecommands;
	for(; i < len; i++){
		if(s[i] == '#'){
			s[i] = NULL;
		}
	}
	removeextrawhitespace(s);
	printf("afterremovewhite%s\n\n",s);
	useablecommands = tokenify(s,";");
	return useablecommands;
}


int 
main(int argc, char **argv) {
    char* prompt = "hitme> ", temp;
	char **commands;
	pid_t* childPIDS;
	char m = 's', mode_ret;
    printf("%s", prompt);
    fflush(stdout);
    char buffer[1024];
	int i = 0, j = 0, numcommands=0,numstrings=0, flag = 0;
    while (fgets(buffer, 1024, stdin) != NULL) {
		char** cmd;
		flag = 0;
        /* process current command line in buffer */
		i = 0; j = 0;
		int len = strlen(buffer);
		buffer[len -1] = '\0';
		temp = buffer;
		printf("temp1 %s",temp);
		cleancolons(temp);
		printf("after cleanc %s",temp);
       // char *cmd[] = { "/bin/ls", "-ltr", ".", NULL };
		commands = useableinput(buffer);
		numcommands = Numstrings(temp, ';');
		printf("%d\n",numcommands);
		if(commands == NULL){
			printf("Error reading input.");
			printf("%s", prompt);
			fflush(stdout);
			continue;
		}
		
				
			while(i < numcommands){
				if(m == 'p'){
					int numchildren = numcommands - i, k = 0;
					childPIDS = malloc(sizeof(pid_t)*numchildren+sizeof(NULL));
					childPIDS[numchildren] = NULL;
					pid_t childpid;
					for(;i<numcommands;i++){
						cmd = tokenify(commands[i], " ");
						numstrings = Numstrings(commands[i], ' ');
						
						if((!strcasecmp(cmd[0], "mode"))||(strlen(cmd[0])==1)){
							mode_ret = mode(cmd, numstrings, m);
							if(mode_ret == m){
								loopfree(cmd, numstrings); numchildren--; 
								childPIDS[numchildren] = NULL; continue;
							}else if(mode_ret == 'f'){
							}else{
								m = mode_ret; loopfree(cmd, numstrings); numchildren--;
								childPIDS[numchildren] = NULL; continue;
							}
						}
						if(!strcasecmp(cmd[0], "exit")){
							flag = 1; numchildren--;
							loopfree(cmd, numstrings); childPIDS[numchildren] = NULL;
						 	continue;
						}
						if((childpid=fork())==0){
							printf("In child: %d with PID: %d\n", j, getpid());
							
							if (execv(cmd[0], cmd) < 0) {
		            			fprintf(stderr, "execv failed: %s\n", strerror	(errno)); 
								loopfree(commands,numcommands);
								loopfree(cmd,numstrings); exit(0);
		        			}

						}else{
							childPIDS[j] = childpid;
						}
						loopfree(cmd, numstrings);
						j++;
					}
					for(; k<numchildren; k++){
						int rstatus=0;
						pid_t child;
						if(childPIDS[k]>0){
							child = waitpid(childPIDS[j],&rstatus,0);
							printf("Parent got carcass of child process %d, return val %d\n", child, rstatus);
							printf("Parallel mode bitch\n");
							assert(childPIDS[k] == child);
							childPIDS[j] = 0;
						}else{
		        			/* fork had an error; bail out */
		        			fprintf(stderr, "fork failed: %s\n", strerror(errno)); exit(0);
		    			}
					}	
					free(childPIDS);
					if(flag > 0){
						loopfree(commands, numcommands);
						time();
						exit(0);
					}
					continue;
				}//End of parallel partition	
					

				else{		
					numstrings = Numstrings(commands[i], ' ');
					printf("useablecmd %s",commands[i]);	
					cmd = tokenify(commands[i], " ");
		
					if((!strcasecmp(cmd[0], "mode"))||(strlen(cmd[0])==1)){
						mode_ret = mode(cmd, numstrings, m);
						if(mode_ret == m){
							i++; loopfree(cmd, numstrings); continue;
						}else if(mode_ret == 'f'){
						}else{
							m = mode_ret; loopfree(cmd, numstrings); break;
						}
					}
					if(!strcasecmp(cmd[0], "exit")){
						flag = 1;
						loopfree(commands,numcommands);
						loopfree(cmd, numstrings);
						time();
						return 0;
					}

			   		pid_t p = fork();
					if (p == 0) {
				    /* in child */
					

				    	if (execv(cmd[0], cmd) < 0) {
				        	fprintf(stderr, "execv failed: %s\n", strerror	(errno)); 
							loopfree(commands,numcommands);
							loopfree(cmd,numstrings); exit(0);
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
					if((!strcasecmp(cmd[0], "exit")) || flag >0){
						flag = 1;
						loopfree(commands, numcommands); 
						loopfree(cmd, numstrings); time(); return 0;
					}
					i++; //next command
					loopfree(cmd, numstrings); 
				}
			}

		if(flag > 0){
			loopfree(commands, numcommands); time();
			return 0;
		}

		if(flag > 0){
			break;
		}

		loopfree(commands, numcommands);
		//all commands have finished
        printf("%s", prompt);
        fflush(stdout);
    }
	loopfree(commands, numcommands);
	time();
    return 0;
}

