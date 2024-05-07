#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "jobs.h"

#define FIFO_NAME "myfifo"
#define SERVER_FILE_NAME "server_file"
#define MAX_MSG_SIZE 512

int main(int argc, char *argv[]) {
    pid_t server_pid;
    int fd;
    char fifo[MAX_MSG_SIZE];

    // Check if the server file exists
    if (access(SERVER_FILE_NAME, F_OK) == -1) {
        // Server file doesn't exist, so fork and create the server
        server_pid = fork();
        if (server_pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (server_pid == 0) {
            // Child process becomes the server
            jobExecutorServer();
            exit(EXIT_SUCCESS);
        }
        // Parent process
        else {
            // Create the FIFO (named pipe) for communication with the server
            if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
                perror("Failed to create FIFO");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Open the FIFO for writing
    if ((fd = open(FIFO_NAME, O_WRONLY)) == -1) {
        perror("Failed to open FIFO for writing");
        exit(EXIT_FAILURE);
    }

    // Construct the message by concatenating command-line arguments
    strcpy(fifo, argv[1]);
    for (int i = 2; argv[i] != NULL; ++i) {
        strcat(fifo, " ");
        strcat(fifo, argv[i]);
    }

    // Write the message to the FIFO
    if (write(fd, fifo, MAX_MSG_SIZE) == -1) {
        perror("Failed to write to FIFO");
        exit(EXIT_FAILURE);
    }

    // Close the FIFO
    close(fd);

    return 0;
}
