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
#define FIFO2_NAME "myfifo2"
#define SERVER_FILE_NAME "server_file"
#define MAX_MSG_SIZE 512

void executeCommand(const char *command) {
    FILE *fp;
    char buffer[MAX_MSG_SIZE];

    // Open a pipe to execute the command
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to execute command");
        exit(EXIT_FAILURE);
    }

    // Read the output of the command
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Send the output to the commander
        response(buffer);
    }

    // Close the pipe
    pclose(fp);
}

void jobExecutorServer() {
    int fd, fd2;
    char msg[MAX_MSG_SIZE];

    // Create FIFOs for communication with the commander
    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
        perror("Failed to create FIFO");
        exit(EXIT_FAILURE);
    }
    if (mkfifo(FIFO2_NAME, 0666) == -1 && errno != EEXIST) {
        perror("Failed to create FIFO");
        exit(EXIT_FAILURE);
    }

    // Open the FIFO for reading
    if ((fd = open(FIFO_NAME, O_RDONLY)) == -1) {
        perror("Failed to open FIFO for reading");
        exit(EXIT_FAILURE);
    }

    // Open the FIFO for writing
    if ((fd2 = open(FIFO2_NAME, O_WRONLY)) == -1) {
        perror("Failed to open FIFO for writing");
        exit(EXIT_FAILURE);
    }

    // Server loop
    while (1) {
        // Read command from the FIFO
        if (read(fd, msg, MAX_MSG_SIZE) == -1) {
            perror("Failed to read from FIFO");
            exit(EXIT_FAILURE);
        }

        // Execute the command
        executeCommand(msg);

        // Send "exit" to indicate the end of the response
        strcpy(msg, "exit");
        if (write(fd2, msg, MAX_MSG_SIZE) == -1) {
            perror("Failed to write to FIFO");
            exit(EXIT_FAILURE);
        }
    }

    // Close the FIFOs
    close(fd);
    close(fd2);
}

void issuejob(char *command, int server_pid) {
    char msg[MAX_MSG_SIZE];

    // Construct the message by appending the command
    snprintf(msg, MAX_MSG_SIZE, "issuejob %s", command);

    // Open a pipe to communicate with the server
    FILE *fp = popen(msg, "r");
    if (fp == NULL) {
        perror("Failed to communicate with server");
        exit(EXIT_FAILURE);
    }

    // Read the response from the server
    char response[MAX_MSG_SIZE];
    while (fgets(response, sizeof(response), fp) != NULL) {
        // Print or handle the response as needed
        printf("%s", response);
    }

    // Close the pipe
    pclose(fp);
}
