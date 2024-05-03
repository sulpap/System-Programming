// gcc pipes_one_terminal.c -o pipes_one_terminal
// ./pipes_one_terminal hello

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_FILE1 "/tmp/pipe1"
#define FIFO_FILE2 "/tmp/pipe2"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <message>\n", argv[0]);
        return 1;
    }

    int fd1, fd2;
    pid_t pid;
    char buf[1024];

    // Create the named pipes (FIFOs)
    mkfifo(FIFO_FILE1, 0666);
    mkfifo(FIFO_FILE2, 0666);

    // Fork the process
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {  // Parent process (Process 1)
        // Open the first named pipe for writing (to send message)
        fd1 = open(FIFO_FILE1, O_WRONLY);
        if (fd1 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Write message to the first named pipe
        write(fd1, argv[1], strlen(argv[1])+1);

        close(fd1);

        // Open the second named pipe for reading (to receive response)
        fd2 = open(FIFO_FILE2, O_RDONLY);
        if (fd2 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Read response from the second named pipe
        read(fd2, buf, sizeof(buf));
        printf("Received response in Process 1: %s\n", buf);

        close(fd2);
    } else {  // Child process (Process 2)
        // Open the first named pipe for reading (to receive message)
        fd1 = open(FIFO_FILE1, O_RDONLY);
        if (fd1 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Read message from the first named pipe
        read(fd1, buf, sizeof(buf));
        printf("Received message in Process 2: %s\n", buf);

        close(fd1);

        // Open the second named pipe for writing (to send response)
        fd2 = open(FIFO_FILE2, O_WRONLY);
        if (fd2 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Write response to the second named pipe
        char *msg2 = "Response from Process 2\n";
        write(fd2, msg2, strlen(msg2)+1);

        close(fd2);
    }

    return 0;
}
