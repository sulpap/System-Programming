// ./main hello

#include "myheaders.h"

char *PIPE1 = "myfifo1";
char *PIPE2 = "myfifo2";
char *server_file = "jobExecutorServer_file";
int SIZE = 400;

int main(int argc, char *argv[]) {

    if (argc == 1) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    int pid2, fd1, fd2;
    pid_t pid;
    char buf[SIZE];
    FILE *sv_file;

    //mkfifo(PIPE1, 0666);

    if(sv_file = open(server_file, O_RDONLY, 0644) == -1) {
        // Fork the process
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0){
            sv_file = fopen(server_file,"w");
            fprintf(sv_file, "%d\n", getpid());
			fclose(sv_file);

        jobExecutorServer();
        } else {
            pid2 = pid;
            if (mkfifo (PIPE1, 0666) < 0){
                if (errno != EEXIST ) {
                    perror ("mkfifo failed") ;
                    exit(1);
                }
            }
        }
    } else {
        fscanf(server_file, "%d", &pid);
		fclose(server_file);
    }
    
    //jobCommander(argv, pid);

    if (pid > 0) {  // Parent process (Process 1)
        // Open the first named pipe for writing (to send message)
        fd1 = open(PIPE1, O_WRONLY);
        if (fd1 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Write message to the first named pipe
        write(fd1, argv[1], strlen(argv[1])+1);

        close(fd1);

        // Open the second named pipe for reading (to receive response)
        fd2 = open(PIPE2, O_RDONLY);
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
        fd1 = open(PIPE1, O_RDONLY);
        if (fd1 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Read message from the first named pipe
        read(fd1, buf, sizeof(buf));
        printf("Received message in Process 2: %s\n", buf);

        close(fd1);

        // Open the second named pipe for writing (to send response)
        fd2 = open(PIPE2, O_WRONLY);
        if (fd2 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        // Write response to the second named pipe
        char *msg2 = "back to you parent\n";
        write(fd2, msg2, strlen(msg2)+1);

        close(fd2);
    }

    return 0;
}
