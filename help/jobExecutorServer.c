#include "myheaders.h"

void jobExecutorServer() {
    
    mkfifo(PIPE2, 0666);

    int fd1, fd2;
    char buf[SIZE];

    // Open the first named pipe for reading (to receive message)
    fd1 = open(PIPE1, O_RDONLY);
    if (fd1 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Read message from the first named pipe
    read(fd1, buf, sizeof(buf));
    printf("Received message in Job Executor Server: %s\n", buf);

    close(fd1);

    // Open the second named pipe for writing (to send response)
    fd2 = open(PIPE2, O_WRONLY);
    if (fd2 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write response to the second named pipe
    char *msg = "Acknowledgement from Job Executor Server\n";
    write(fd2, msg, strlen(msg) + 1);

    close(fd2);
}
