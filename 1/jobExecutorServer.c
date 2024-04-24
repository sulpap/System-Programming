#include "includes.h"

#include <sys/wait.h>
#include <signal.h>

extern char *WRITE_PIPE;
extern char *READ_PIPE;
extern int SIZE;

void jobExecutorServer() {
    int fd, id = 0;
    char buffer[SIZE + 1], response[SIZE];
    char *operation, *parameter;

    // Εγκατάσταση σήματος για το SIGCHLD
    //signal(SIGCHLD, handle_sigchld);

    // we make the pipe to send back response to commander
    if( mkfifo(READ_PIPE, 0666) == -1 ){
        if (errno != EEXIST ) {
            perror ("mkfifo failed") ;
            return;
        }
    }

    for (;;) {
        // we open the pipe for reading
        fd = open(WRITE_PIPE, O_RDONLY);
        if (fd == -1) {
            perror("Server reading: pipe open error");
            return 1;
        }

        // we read the command from the pipe
        if (read(fd, buffer, SIZE+1) == -1) {
            perror("Server reading: read error");
            return 1;
        }

        // closing reading pipe
        close(fd);

        // Διαχωρισμός του μηνύματος σε λειτουργία και παραμέτρους
        operation = strtok(buffer, " ");
        parameter = strtok(NULL, "");

        // Ανάλυση της λειτουργίας και εκτέλεση αντίστοιχης ενέργειας
        switch(operation[0]) {
            case 'issueJob':
                //unknown_command = 0;
                if (parameter == NULL) {
                    printf("\"issueJob\" : missing argument\n");
                } else {
                    //issueJob(parameter, &running_jobs_list, &queued_jobs_list, jobID, 1);
                    //jobID++;
                }
                break;
            
            case 'setConcurrency':

            case 'stop':

            case 'poll':

            case 'exit':

            default:

        }








        // // Ανοίγουμε το pipe για εγγραφή απαντήσεων
        // fd2 = open(READ_PIPE, O_WRONLY);
        // if (fd2 == -1) {
        //     perror("Server writing: pipe open error");
        //     return 1;
        // }

        // // Εκτέλεση της εντολής
        // int pid = fork();
        // if (pid == 0) { // Παιδί
        //     // Εκτέλεση της εντολής
        //     system(buffer);
        //     // Εγγραφή απάντησης στο pipe
        //     write(fd2, "Command executed successfully.", strlen("Command executed successfully.") + 1);
        //     // Έξοδος από το παιδί
        //     exit(0);
        // } else if (pid > 0) { // Γονέας
        //     // Κλείσιμο pipe εγγραφής
        //     close(fd2);
        // } else { // Σφάλμα
        //     perror("Fork error");
        //     return 1;
        // }
    }
}