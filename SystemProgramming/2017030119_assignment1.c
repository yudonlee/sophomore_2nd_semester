#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
int main(int argc,char** argv){
	//if argc <2 ,then we doesn't get command ls.so print "enter command".
	if(argc <2){
		fprintf(stderr,"Please enter command\n");
		exit(1);
	}
	//if argv[1] is not equal to ls,then we should not execute ls command.
	if(strcmp(argv[1],"ls")){
		fprintf(stderr,"Please input right command ""ls""\n");
		exit(1);
	}
	//execv function converts current process to file process,so I make child process and throw child process to pointed file process.
	int status;
	pid_t pid = fork();
	//use command line parameter to execute execv function parameter
	char **new_argv = (char **)malloc(sizeof(char*)*(argc+1));
	for(int idx = 1;idx<argc; idx++)
		new_argv[idx-1] = argv[idx];
	//last parameter must null.
	new_argv[argc] = NULL;
	if(pid<0){
		perror("fork error!");
	}
	else if(pid==0){
		 //execv need NULL as last parameter.(reference Linux manpage)the list of argument must be terminated by NULL pointer.these string shall constitute the argument list availabe to the new process.so the list must be terminated by null Pointer. 
		//if execv function is failed,then return -1.so this if statement handle them.
		if(execv("/bin/ls",new_argv)<0){
			fprintf(stderr,"Error execution!\n");
			exit(1);
		}
	}
	else{	
		wait(&status); //wait for child process exit
		exit(1);  //parent process is terminated.
	}
	return 0;
}
