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
	pid_t child, pchild;
	int rstatus=0, status=0, PIDret;
	char* input[]={"/bin/ls", NULL};
	struct pollfd pfds[1];
	struct timespec time;
	time.tv_sec = 3; time.tv_nsec = 0;
	char buffer[1000];
	child = fork();
	if(child ==0){
		int i = 0;
		poll(NULL,0,5000);
		exit(3);
		printf("IN child: %d\n",getpid());
		if (execv(input[0], input) < 0) {
		 	fprintf(stderr, "execv failed: %s\n", strerror	(errno));
		}
	}
	pfds[0].fd = 0;
	pfds[0].events = POLLIN;
	while(1){
		status = poll(pfds,1,1000);
		if(status == 0){
			PIDret = waitpid(child,&rstatus,WNOHANG);
			printf("PIDret: %d %d\n",PIDret,WIFEXITED(rstatus));
			if(PIDret == child){
				printf("WIFEXITED: %d\n",WIFEXITED(rstatus));
				printf("yay it exited normally\n"); break;}
		printf("not ready yet\n");
		}
		if(status >0){
			printf("stop typing bitch!\n");
			fflush(stdout);
			//fgets(buffer,1000,stdin);
		}}
	printf("returning child: %d\n",PIDret);
	printf("Finally\n");
/*else if(child >0){
		pchild = wait(&rstatus);
		assert(pchild==child);
		printf("parent of child: %d\n",child); 
	}*/
	return 0;
}
