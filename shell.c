// C Program to design a shell in Linux 
#include<stdio.h>
#include <sys/types.h>
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include<readline/readline.h> 
#include<signal.h>
#include <string.h> 

#define MAXCOM 515 // max number of letters to be supported 
#define MAXLIST 100 // max number of commands to be supported 
#define NoOfOwnCmds 5 // number of my own commands


int h = 0, cPressed = 0, piped = 0 ;
char* history[MAXLIST];
pid_t p1, p2, pid; 
FILE *fp;
int writeMsg(char* message){
    int fd;
    char * myfifo = "myfifo";

    // create the FIFO (named pipe)
    mkfifo(myfifo, 0666);

    // write to the FIFO
    fd = open(myfifo, O_WRONLY);
    write(fd, message, sizeof(message));
    close(fd);

    // remove the FIFO
    unlink(myfifo);
//check
//printf("write end\n");
    return 0;
}
int readMsg(){
    int fd;
    char * myfifo = "myfifo";
    char buf[MAXCOM];

    //open, read, and display the message from the FIFO 
    fd = open(myfifo, O_RDONLY);
    read(fd, buf, MAXCOM);
    if(read)
	printf("\n\nprompt>>> New Message: %s\n", buf);
    close(fd);
//check
//printf("read end\n");
    return 0;
}





// Function where the system command is executed 
void execArgs(char** parsed){ 
	// Forking a child 
	pid = fork(); 

	if (pid == -1) { 
		printf("\nFailed forking child.."); 
		return; 
	}
	else if (pid == 0) { 
		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command.."); 
		} 
		exit(0); 
	}
	else { 
		// waiting for child's terminate 
		wait(NULL); 
		return; 
	} 
} 

//for pipe command
void execArgsPiped(char** parsed, char** parsedpipe) { 
	int pipefd[2]; 


	if (pipe(pipefd) < 0) { 
		printf("\nPipe could not be initialized"); 
		return; 
	} 
	p1 = fork(); 
	if (p1 < 0) { 
		printf("\nCould not fork"); 
		return; 
	} 

	if (p1 == 0) { 
		// Child 1 executing.. 
		close(pipefd[0]); //read end
		dup2(pipefd[1], STDOUT_FILENO); //write at the write end
		close(pipefd[1]); //write end 

		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command 1.."); 
			exit(0); 
		} 
	} else { 
		// Parent executing 
		p2 = fork(); 

		if (p2 < 0) { 
			printf("\nCould not fork"); 
			return; 
		} 

		// Child 2 executing.. 
		// It only needs to read at the read end 
		if (p2 == 0) { 
			close(pipefd[1]); 
			dup2(pipefd[0], STDIN_FILENO); 
			close(pipefd[0]); 
			if (execvp(parsedpipe[0], parsedpipe) < 0) { 
				printf("\nCould not execute command 2.."); 
				exit(0); 
			} 
		} else { 
			// parent executing, waiting for two children 
			wait(NULL); 
			wait(NULL); 
		} 
	} 
} 


// Function to execute builtin commands 
int ownCmd(char** parsed) { 
	int i, switchOwnArg = -1; 
	char* ListOfOwnCmds[NoOfOwnCmds], cwd[1024];

	ListOfOwnCmds[0] = "quit"; 
	ListOfOwnCmds[1] = "cd"; 
	ListOfOwnCmds[2] = "help"; 
	ListOfOwnCmds[3] = "msg"; 
	ListOfOwnCmds[4] = "history";
	ListOfOwnCmds[5] = "getDir";

	for (i = 0; i < NoOfOwnCmds; i++)
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0)
			switchOwnArg = i; 

	switch (switchOwnArg) { 
	case 0: 
		//end the shell
		printf("\nGoodbye\n"); 
		exit(0); 
	case 1: 
		//call cd in kernel
		chdir(parsed[1]); 
		return 1; 
	case 2: 
		// Help command -> type my commands
		printf("\n***WELCOME TO SHAMIM SHELL HELP***\nList of Commands supported:\n>cd\n>ls\n>quit\n>history\n>msg\ngetDir\n>all other general commands available in shell\n>pipe handling\n>improper space handling"); 
		return 1; 
	case 3: //message to other kernel
		writeMsg(parsed[1]);
		return 1; 
	case 4:
		//show history
		for(int i = 0; i < h ; i ++)
			printf("\nprompt>>> %s", history[i]);
		return 1;
	case 5:
		//print Current Directory
		getcwd(cwd, sizeof(cwd)); 
		printf("\nDir: %s", cwd);
		return 1;
	default: 
		break; 
	} 
	//when input is a kernel command
//check
//printf("own end\n");
	return 0; 
} 


// function for finding pipe 
int parsePipe(char* str, char** strpiped) { 
	int i; 
	for (i = 0; i < 2; i++) { 
		strpiped[i] = strsep(&str, "|"); 
		if (strpiped[i] == NULL) 
			break; 
	} 

	if (strpiped[1] == NULL) 
		return 0; // returns zero if no pipe is found. 
	else
		return 1; 
} 


// function for parsing command words 
void parseSpace(char* str, char** parsed) { 
	int i; 

	for (i = 0; i < MAXLIST; i++) { 
		parsed[i] = strsep(&str, " "); 
		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
} 


int processString(char* str, char** parsed, char** parsedpipe) { 

	char* strpiped[2]; 

	piped = parsePipe(str, strpiped); 

	if (piped) { 
		parseSpace(strpiped[0], parsed); 
		parseSpace(strpiped[1], parsedpipe); 

	} else { 

		parseSpace(str, parsed); 
	} 

	if (ownCmd(parsed)) 
		return 0; 
	else
		return 1 + piped; 
} 


// Function to take input 
int takeInput(char* str, int args,char filename[100]) { 
	char* buf; 
	size_t bufsize;
	if(args == 1){
		buf = readline("\nprompt>>> "); 	
	}
	else{
		fp = fopen(filename,"r");
		if (!fp){
       			printf("batch file not exist!");
			exit(0);
		}
		if(!getline(&buf, &bufsize, fp)){
			printf("batch file ends!");
			exit(0);
		}
		printf("\n%s", buf);
		
	}

	if (strlen(buf) != 0) { 
		//add_history(buf); 
		//save in history
		history[h] = buf;
		h ++;
		strcpy(str, buf); 
		return 0; 
		//enter pressed
		if(strlen(buf) == 1 && (int)buf[0] == 13)
			return 3;
	
		//ctrl + d pressed
		if(strlen(buf) == 1 && (int)buf[0] == 4){
			printf("\nGoodbye\n"); 
			sleep(0.5);
			exit(0);
		}
		//ctrl + c pressed
		if(strlen(buf) == 1 && (int)buf[0] == 3){
			printf("%s stop!", history[h-1]);
			return 3;
		}
		if(strlen(buf) > 512)
			return 2;
	}
//check
//printf("take end\n");
 return 1; 
} 


void ctrlCPressed(int x){
	cPressed = 1;
	return;
}


int main(int args, char **argv) { 
	char cwd[1024], inputString[MAXCOM], *parsedArgs[MAXLIST], *parsedArgsPiped[MAXLIST], *username; 


	signal(SIGINT, ctrlCPressed);

	int execFlag = 0, in; 


	//show start page of shell
	//clear screen
	system("clear");
	printf("\n\n\n\n******************"
		"************************"); 
	printf("\n\n\n\t****SHAMIM SHELL****"); 
	printf("\n\n\n\n*******************"
		"***********************"); 
	username = getenv("USER"); 
	printf("\n\n\nUSER is: @%s\n", username); 
	sleep(2); 
	system("clear");

	while (1) { 
		//print Current Directory
		getcwd(cwd, sizeof(cwd)); 
		printf("\nDir: %s", cwd); 

		//check for ctrl + c pressed -> stop process and start again
		if(cPressed){
			cPressed = 0;
			continue;
		}
		if(h > 0 && !strstr(history[h-1],"msg"))
			readMsg();
		// take input 
		in = takeInput(inputString, args,argv[1]);		
		//empty input
		if (in == 1) {
			printf("there is no command to run");
			continue; 			
		}
		// too long input
		if (in == 2) {
			printf("the command is too long");
			continue; 			
		}
		if (in == 3)
			continue; 			

		// process 
		execFlag = processString(inputString, 
		parsedArgs, parsedArgsPiped); 
		// execflag returns zero if there is no command 
		// or it is a builtin command, 
		// 1 if it is a simple command 
		// 2 if it is including a pipe. 

		// execute 
		if (execFlag == 1) 
			execArgs(parsedArgs); 

		if (execFlag == 2) 
			execArgsPiped(parsedArgs, parsedArgsPiped); 

	} 
	return 0; 
} 

