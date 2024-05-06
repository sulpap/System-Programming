#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>

#define MAXLEN 1024 // Μέγιστο μήκος ενός μηνύματος
#define BUF_SIZE 1024 // Μέγεθος buffer

int main(int argc, char *argv[]) {
    const char *fifo = "mypipe";
    const char *serverinfo = "./serverinfo";
    char buffer[BUF_SIZE];
    int pd, fd;
    pid_t serverPid;

    // Άνοιγμα του named pipe για ανάγνωση/εγγραφή
    if ((pd = open(fifo, O_RDWR | O_NONBLOCK)) < 0) {
        perror("Error opening pipe");
        exit(1);
    }

    // Άνοιγμα ή δημιουργία του αρχείου πληροφοριών του server
    if ((fd = open(serverinfo, O_RDONLY, 0644)) == -1) {
        perror("Error opening serverinfo file");
        exit(1);
    }

    // Ανάγνωση του PID του server από το αρχείο πληροφοριών
    if (read(fd, buffer, BUF_SIZE) == -1) {
        perror("Error reading server PID");
        exit(1);
    }
    close(fd);

    serverPid = atoi(buffer);

    // Καθορισμός του τύπου εντολής που θα εκτελεστεί
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [OPTION] [ARGUMENT]\n", argv[0]);
        exit(1);
    }

    // Αποστολή της εντολής στον server
    if (write(pd, argv[1], strlen(argv[1])) == -1) {
        perror("Error writing to pipe");
        exit(1);
    }

    // Στέλνεται SIGCONT στον server για να επεξεργαστεί την εντολή
    kill(serverPid, SIGCONT);

    // Αναμονή για απάντηση από τον server
    if (read(pd, buffer, BUF_SIZE) == -1) {
        perror("Error reading from pipe");
        exit(1);
    }

    // Εκτύπωση της απάντησης
    printf("%s\n", buffer);

    close(pd);
    return 0;
}


int jobExecutorServer() {
    const char *fifo = "mypipe";
    const char *serverinfo = "./serverinfo";
    char buffer[BUF_SIZE];
    int pd, fd;

    // Άνοιγμα του named pipe για ανάγνωση/εγγραφή
    if ((pd = open(fifo, O_RDWR | O_NONBLOCK)) < 0) {
        perror("Error opening pipe");
        exit(1);
    }

    // Δημιουργία ή άνοιγμα του αρχείου πληροφοριών του server
    if ((fd = open(serverinfo, O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1) {
        perror("Error opening serverinfo file");
        exit(1);
    }

    // Εγγραφή του PID του server στο αρχείο πληροφοριών
    snprintf(buffer, BUF_SIZE, "%d", getpid());
    if (write(fd, buffer, strlen(buffer)) == -1) {
        perror("Error writing server PID");
        exit(1);
    }
    close(fd);

    // Αναμονή για εντολές από τον jobCommander
    for (;;) {
        if (read(pd, buffer, BUF_SIZE) == -1) {
            perror("Error reading from pipe");
            exit(1);
        }

        // Εδώ μπορείτε να εκτελέσετε κάποια εντολή ανάλογα με το περιεχόμενο του buffer

        // Παράδειγμα: απλή εκτύπωση της εντολής
        printf("Received command: %s\n", buffer);
    }

    close(pd);
    return 0;
}
