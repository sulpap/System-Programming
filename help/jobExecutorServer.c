#include "myheaders.h"

void jobExecutorServer() {
    // int fd1, fd2;
    // char buf[SIZE + 1];

    mkfifo(PIPE2, 0666);

    // // Open the first named pipe for reading (to receive message)
    // fd1 = open(PIPE1, O_RDONLY);
    // if (fd1 == -1) {
    //     perror("Server reading - pipe open error");
    //     exit(EXIT_FAILURE);
    // }

    // // Read message from the first named pipe
    // read(fd1, buf, sizeof(buf));
    // printf("Received message in Process 2: %s\n", buf);

    // close(fd1);

    // // Open the second named pipe for writing (to send response)
    // fd2 = open(PIPE2, O_WRONLY);
    // if (fd2 < 0) {
    //     perror("open");
    //     exit(EXIT_FAILURE);
    // }

    // // Write response to the second named pipe
    // char *msg2 = "back to you parent\n";
    // write(fd2, msg2, strlen(msg2)+1);

    // close(fd2);

}