// gcc writer.c -o writer
// ./writer

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_FILE "/tmp/myfifo"

int main() {
    int fd;
    char *msg = "Hello from Terminal 1\n";

    // Create the named pipe (FIFO)
    mkfifo(FIFO_FILE, 0666);

    // Open the named pipe for writing
    fd = open(FIFO_FILE, O_WRONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write message to the named pipe
    write(fd, msg, strlen(msg)+1);
    close(fd);

    return 0;
}
