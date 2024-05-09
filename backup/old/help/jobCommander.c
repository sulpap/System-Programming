#include "myheaders.h"

int jobCommander(int argc, char *argv[]) {
    int pid, fd1, fd2;
    char buf[SIZE];

    // Open the second named pipe for reading (to receive message)
    fd1 = open(PIPE2, O_RDONLY);
    if (fd1 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Read message from the second named pipe
    read(fd1, buf, sizeof(buf));
    printf("Received message in Job Commander: %s\n", buf);

    close(fd1);

    // Open the first named pipe for writing (to send response)
    fd2 = open(PIPE1, O_WRONLY);
    if (fd2 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write response to the first named pipe
    char *msg = "Acknowledgement from Job Commander\n";
    write(fd2, msg, strlen(msg) + 1);

    close(fd2);

    return 0;
}
