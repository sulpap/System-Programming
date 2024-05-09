#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "jobs.h"


void jobCommander(char **msg, int server_pid) {
	int fd, fd2, i;
	char msgbuf[MSGSIZE+1]; //msgb[MSGSIZE+1];

	/*open the pipe to read data*/
	fd=open(PIPE1, O_WRONLY);
	if( fd == -1) {
		perror("commander writing: fifo open error");
		exit(1);
	}

	i = 2;
        /*attach all the arguments of argv in a string*/
	//strcpy(msgbuf, msg[1]);
	sprintf(msgbuf, "%s", msg[1]);
	while (msg[i] != NULL) {
		sprintf(msgbuf, "%s %s", msgbuf, msg[i]);
		i++;
	}
	//length = strlen(msgbuf);
	
	/*if ((nwrite=write(fd, &length, sizeof(int))) == -1) {
                perror("Commander : Error in Writing");
                exit(2);
        }*/	

        /*write the string in the pipe*/  
	if (write(fd, msgbuf, MSGSIZE + 1) < 0) {
		perror("Commander : Error in Writing");
		exit(1);
	}
	close(fd);
	
 	/*open the second pipe for reading response*/
	fd2=open(PIPE2, O_RDWR);
	if ( fd2 < 0) {
		perror("commander reading : fifo open problem"); 
		exit(1);	
	}
	/*while server is sending something back*/
	while (1) {
		ssize_t bytes_read = read(fd2, msgbuf, MSGSIZE+1);
		/*read response*/
		if (bytes_read < 0) {
			perror("commander : problem in reading");
			exit(1);
		}
		/*if i read "exit" this means the end of arguments*/
		else {
            // Εκτυπώνουμε την απάντηση που λάβαμε
            //msgbuf[bytes_read] = '\0'; // Τερματίζουμε το string με NULL χαρακτήρα

            // Έλεγχος για το "exit" για τερματισμό του βρόχου
            if (strcmp(msgbuf, "exit") == 0) {
                break;
            }
            printf("Response from server: %s\n", msgbuf);
        }			
		

	}
	close(fd2);
	
}
	
	
	
