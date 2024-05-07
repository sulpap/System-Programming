#include "myheaders.h"
#include "list.h"

#include <sys/wait.h>
#include <signal.h>

void parser(char *command, char **args);
void handler(int sig);

void issueJob(char *command, Queue* running, Queue* queued, int *jobID, int *queuePosition) {
    char *args[64], response[SIZE];
    int running_num;

    parser(command, args); // analysh ths entolhs se orismata

    // Σήμανση για αντιμετώπιση SIGCHLD σε περίπτωση που έχει τελειώσει κάποια διεργασία
    signal(SIGCHLD, handler); // in case a process ends

    running_num = queueSize(running);

    // Υπάρχει χώρος για περισσότερες εργασίες που τρέχουν
    if (running_num < Concurrency) {
        if (running_num > 0) {
            // Το flag είναι 1 όταν η συνάρτηση καλείται γιατί μια εργασία έχει τελειώσει και μια εκκρεμής θα μεταφερθεί στις τρέχουσες
            sprintf(response, "job_%d\t%s\tRUNNING", *jobID, command);
            respond_string(response);
        } else {
            printf("job_%d\t%s\tRUNNING\n", *jobID, command);
        }

        pid_t pid = fork();
        if (pid < 0) { 
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) { 
            // execute command
            execvp(*args, args); 
            perror(*args);
            exit(EXIT_FAILURE);
        } else { 
            // add process to running jobs
            add(running, strdup(command), jobID);
        }
    } else {
        // den yparxei xwros opote th vazume sthn oura anamonhs
        sprintf(response, "job_%d\t%s\tQUEUED", *jobID, command);
        respond_string(response);
        add(queued,strdup(command), jobID);
    }

    (*jobID)++;
    (*queuePosition)++;
}

// Συνάρτηση για τη διαχωρισμό μιας γραμμής εντολών σε ορίσματα
void parser(char *command, char **args) {
    char *token = strtok(command, " \t\n"); // tokenize based on spaces, tabs and newlines
    int i = 0;

    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }

    args[i] = NULL;
}

// Συνάρτηση που χειρίζεται το SIGCHLD σήμα
void handler(int sig) {
    pid_t terminated_pid;
    int status, running_num, queued_num, i, capacity;
    char job[200];

    // Χρήση της waitpid για την αναμονή του παιδιού
    terminated_pid = waitpid(-1, &status, WNOHANG);
    while (terminated_pid > 0) {
        delete_job(&running, pid);
    }

    running_num = queueSize(running);
    //printf("Child process with PID %d terminated\n", pid);

    if (running_num < Concurrency) {
        //capacity = Concurrency - running_num;
        while (!isEmpty(queued) && running_num < Concurrency) {
            queued_num = queueSize(queued);
            if (queued>0) {
                Queue tmp = queued;
                int jobID = tmp->jobID;
                strcpy(job, tmp->job);

                int qid = get_first_job(queued);
                delete_job(queued, qid);
                issuejob(job, &running, &queued, jobID, 0);
            }
        }
    }
    //if there is space in the list
    //try to run as many jobs as the running list can take
    // as long as there are queued jobs
    // if queued > 0 remove the oldest queued job



}


