#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "myheaders.h"

void jobCommander(char **msg, int server_pid) {
    int fd1, fd2;
    char buf[SIZE+1];

    fd1 = open(PIPE1, O_WRONLY);
    if (fd1 == -1) {
        perror("Commander writing: pipe open error");
        exit(EXIT_FAILURE);
    }

    // Write message to the first named pipe
    int i = 1; // Ξεκινάμε από το δεύτερο στοιχείο του πίνακα msg
    // Αντιγράφουμε την πρώτη παράμετρο
    sprintf(buf, "%s", msg[1]);
    // Συνεχίζουμε να προσθέτουμε τις υπόλοιπες παραμέτρους στο buf
    while (msg[i] != NULL) {
        sprintf(buf, "%s %s", buf, msg[i]);
        i++;
    }

    // we write the string in the pipe
    if( (write(fd1,buf,SIZE+1)) == -1 ){
        perror("Commander writing: write error");
        exit(EXIT_FAILURE);
    }

    // Open the second named pipe for reading (to receive response)
    fd2 = open(PIPE2, O_RDONLY);
    if (fd2 == -1) {
        perror("Commander reading: pipe open error");
        exit(EXIT_FAILURE);
    }

    // Read response from the second named pipe
    read(fd2, buf, sizeof(buf));
    printf("Received response in Process 1: %s\n", buf);

    close(fd2);

}