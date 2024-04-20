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
	int fd, fd2, nwrite, unknown_command = 1, i, length;
	char msgbuf[MSGSIZE+1], msgb[MSGSIZE+1];

	//open the pipe to read data
	fd = open(fifo, O_WRONLY);
	if( fd < 0 ) 
	{
		perror("Commander writing: fifo open error");
		exit(1);
	}

	i = 2;
	//attach all the arguments of argv in a string
	strcpy(msgbuf, msg[1]);
	while (msg[i] != NULL) 
	{
		sprintf(msgbuf, "%s %s", msgbuf, msg[i]);
		i++;
	}


	//length = strlen(msgbuf);
	
	/*if ((nwrite=write(fd, &length, sizeof(int))) == -1) {
                perror("Commander : Error in Writing");
                exit(2);
        }
	*/	



    //write the string in the pipe
	nwrite = write(fd, msgbuf, MSGSIZE + 1);
	if ( nwrite == -1 ) 
	{
		perror("Commander : Error in Writing");
		exit(2);
	}
	close(fd);
	
 	//open the second pipe for reading response
	fd2 = open(fifo2, O_RDWR);
	if ( fd2 < 0 ) 
	{
		perror("commander reading : fifo open problem"); 
		exit(3);	
	}

	//while server is sending something back
	while (1) 
	{
		//read response
		if ( read(fd2, msgb, MSGSIZE+1) < 0 ) 
		{
			perror("commander : problem in reading");exit(5);
		}

		//if I read "exit" this means the end of arguments
		if ( strcmp(msgb, "exit") == 0 )
			break;
		
		//print response
		printf("%s\n", msgb);
	}
	close(fd2);
}
	
	
	