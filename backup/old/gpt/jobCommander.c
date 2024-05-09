#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_NAME "myfifo"
#define MAX_MSG_SIZE 512

void jobCommander(char **msg, int server_pid) {
    int fd;
    char fifo[MAX_MSG_SIZE];

    // Open the named pipe for writing
    if ((fd = open(FIFO_NAME, O_WRONLY)) == -1) {
        perror("Failed to open FIFO for writing");
        exit(EXIT_FAILURE);
    }

    // Construct the message by concatenating command-line arguments
    strcpy(fifo, msg[1]);
    for (int i = 2; msg[i] != NULL; ++i) {
        strcat(fifo, " ");
        strcat(fifo, msg[i]);
    }

    // Write the message to the named pipe
    if (write(fd, fifo, MAX_MSG_SIZE) == -1) {
        perror("Failed to write to FIFO");
        exit(EXIT_FAILURE);
    }

    // Close the named pipe
    close(fd);
}
