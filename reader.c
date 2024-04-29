#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_FILE "/tmp/myfifo"

int main() {
    int fd;
    char buf[1024];

    // Open the named pipe for reading
    fd = open(FIFO_FILE, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Read from the named pipe
    read(fd, buf, sizeof(buf));
    printf("Received message: %s", buf);

    close(fd);

    return 0;
}
