// #include "includes.h"

// char *WRITE_PIPE = "myfifo";
// char *READ_PIPE = "myfifo2";
// int SIZE = 512;
// char *jobExecutorServer_file = "jobExecutorServer_file";  

// int main(int argc, char *argv[]) {
//     // Ελέγχουμε αν έχουμε τουλάχιστον δύο ορίσματα για το πρόγραμμα
//     if (argc < 3) {
//         printf("Usage: %s <message...>\n", argv[0]);
//         return 1;
//     }

//     // Καλούμε τη συνάρτηση jobCommander με τα ορίσματα που δίνονται από τη γραμμή εντολών
//     jobCommander(argv, getpid());

//     return 0;
// }

void jobExecutorServer() {
    int fd, unknown_command = 1, jobID = 0;
    char msgbuf[MSGSIZE + 1], *operation, *parameter, response_string[100];

    // Δημιουργία named pipes
    if (mkfifo(MY_FIFO_NAME, 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo failed");
            exit(6);
        }
    }

    for (;;) {
        // Άνοιγμα named pipe για ανάγνωση
        if ((fd = open(MY_FIFO_NAME, O_RDONLY)) < 0) {
            perror("server: fifo open problem");
            exit(3);
        }

        // Διάβασμα μηνύματος από το named pipe
        if (read(fd, msgbuf, MSGSIZE + 1) < 0) {
            perror("problem in reading");
            exit(5);
        }
        close(fd);

        // Διαχωρισμός του μηνύματος σε λειτουργία και παραμέτρους
        operation = strtok(msgbuf, " ");
        parameter = strtok(NULL, "");

        // Ανάλυση της λειτουργίας και εκτέλεση αντίστοιχης ενέργειας
        switch(operation[0]) {
            case 'i': // issueJob
                unknown_command = 0;
                if (parameter == NULL) {
                    printf("\"issueJob\" : missing argument\n");
                } else {
                    issueJob(parameter, &running_jobs_list, &queued_jobs_list, jobID, 1);
                    jobID++;
                }
                break;
            case 's': // setConcurrency or stop
                if (operation[3] == 'C') { // setConcurrency
                    unknown_command = 0;
                    int N = atoi(parameter);
                    sprintf(response_string, "%s %d", "Concurrency changed to :", N);
                    response(response_string);
                    update_running();
                } else if (operation[3] == 'p') { // stop
                    unknown_command = 0;
                    int id = atoi(parameter);
                    if (!stop(&running_jobs_list, id)) {
                        if (!stop_queued(&queued_jobs_list, id)) {
                            sprintf(response_string, "%s", "No job found to stop");
                            response(response_string);
                        } else {
                            sprintf(response_string, "%s %d", "Job stopped :", id);
                            response(response_string);
                        }
                    } else {
                        sprintf(response_string, "%s %d", "Job stopped :", id);
                        response(response_string);
                    }
                }
                break;
            case 'p': // poll
                unknown_command = 0;
                if (strcmp(parameter, "running") == 0) {
                    char *args[64];
                    print_running(running_jobs_list, args);
                    send_response(args);
                } else if (strcmp(parameter, "queued") == 0) {
                    char *args[64];
                    print_queued(queued_jobs_list, args);
                    send_response(args);
                }
                break;
            case 'e': // exit
                unknown_command = 0;
                exit_operation(fd);
                break;
            default:
                sprintf(response_string, "%s", "Operation selected : unknown\n");
                response(response_string);
        }

        fflush(stdout);
    }
}
