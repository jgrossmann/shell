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

int main(void){
	struct rusage usage;
	struct timeval usertime, kerneltime;
	char* string1 = malloc(sizeof(char)*5+sizeof(NULL));
	char string2[] = {"hello"};
	strcpy(string1,string2); string1[5] = NULL;
	printf("%s\n",string1);
	pid_t* pids = malloc(sizeof(pid_t)*5 + sizeof(NULL));
	int a = 1;
	pid_t p;
	pids[5] = NULL;
	int i = 0, j = 0;
	for(; i<5;i++){
		if((p=fork())==0){
			printf("In child: %d PID: %d\n",i,getpid());
			a++;
			exit(0);
		}
		else{
			pids[i] = p;
		}
	}
	
	for(;j<5;j++){
		int rstatus=0;
		pid_t child;
		if(pids[j]>0){
			child = waitpid(pids[j],&rstatus,0);
			printf("parent picked up child: %d with PID:%d\n",j,child);
			printf("%d\n",rstatus);
			pids[j] = 0;
		}
	}
	int k = 0;
	for(; k<10000; k++){}
	sleep(1);
	printf("%s\n",string2);
	free(pids); free(string1);
	getrusage(RUSAGE_SELF, &usage);	
	usertime=usage.ru_utime; kerneltime=usage.ru_stime;
	printf("user: %ld.%08lds, kernel: %ld.%08lds\n",usertime.tv_sec,usertime.tv_usec, kerneltime.tv_sec,kerneltime.tv_usec); 
	return 0;
}
