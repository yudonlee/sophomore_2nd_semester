#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
int main(int argc,char** argv){
	if(argc <2){
		fprintf(stderr,"Please enter command\n");
		exit(1);
	}
	if(strcmp(argv[1],"ls")){
		fprintf(stderr,"Please input right command ""ls""\n");
		exit(1);
	}
	pid_t pid = fork();
	if(pid<0){
		perror("fork error!");
	}
	else if(pid==0){
		argv[argc] = (char*)0; //execv need NULL as last parameter.(reference Linux manpage)the list of argument must be terminated by NULL pointer.these string shall constitute the argument list availabe to the new process.so the list must be terminated by null Pointer. 
		if(execv("/bin/ls",argv+1)<0)
			fprintf(stderr,"Error execution!\n");
	}
	else{	
		wait(0); //wait for child process exit
		exit(1);
	}
	return 0;
}
