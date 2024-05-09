#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "jobs.h"

char *PIPE1 = "myfifo1";
char *PIPE2 = "myfifo2";
char *server_file_name = "server_file";
int MSGSIZE = 1024;

int main(int argc, char *argv[]) {

	if (argc == 1) {
		printf("Wrong number of arguments\n");
		exit(1);
	}

	//pid_t server_pid;
	FILE *server_file;
	//FILE *commander_file;
	int fd, pid;

	server_file = fopen(server_file_name, "r");
        /*the file does not exists so i need to create the server*/
	if (server_file == NULL) {
		pid_t server_pid = fork();
		if (server_pid < 0) {
			perror("Fork failed");
			exit(1);
		}

		if (server_pid == 0) {
	                /*server : write pid to file*/
			server_file = fopen(server_file_name, "w");
			fprintf(server_file, "%d\n", getpid());
			fclose(server_file);
			jobExecutorServer();
		}
		else {
			pid = server_pid;
                        /*create PIPE1 - named pipe*/
			if (mkfifo (PIPE1 , 0666) < 0){
				if (errno != EEXIST ) {
					perror ("mkfifo failed") ;
					exit(1) ;
				}
			}
		}
	}
/*file exists so i just copy the pid from it*/
	else {
		fscanf(server_file, "%d", &pid);
		fclose(server_file);
	}
	
	jobCommander(argv, pid);

}
