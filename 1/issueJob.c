#include "myheaders.h"
#include "list.h"

#include <sys/wait.h>
#include <signal.h>

void parse(char *command, char **args);
void handler(int sig);


// void issueJob(char *command, Queue* running, Queue* queued, int *jobID, int *queuePosition) {
//     printf("issue job is running");
// }


void issueJob(char *command, Queue* running, Queue* queued, int *jobID, int *queuePosition) {
    char *args[64], response[SIZE];
    int running_jobs_count;

    // Ανάλυση της εντολής σε ορίσματα
    parse(command, args);

    // Σήμανση για αντιμετώπιση SIGCHLD σε περίπτωση που έχει τελειώσει κάποια διεργασία
    signal(SIGCHLD, handler);

    // Μέτρηση των εργασιών που τρέχουν
    running_jobs_count = queueSize(running);

    // Υπάρχει χώρος για περισσότερες εργασίες που τρέχουν
    if (running_jobs_count < Concurrency) {
        if (running_jobs_count > 0) {
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
            // Εκτέλεση της εντολής
            execvp(*args, args); 
            perror(*args);
            exit(EXIT_FAILURE);
        } else { 
            // Προσθήκη της εργασίας στις τρέχουσες
            add(running, jobID, strdup(command), *queuePosition);
        }
    } else {
        // Δεν υπάρχει χώρος για περισσότερες τρέχουσες εργασίες, οπότε προστίθεται στις εκκρεμείς
        sprintf(response, "job_%d\t%s\tQUEUED", *jobID, command);
        respond_string(response);
        add(queued, jobID, strdup(command), *queuePosition);
    }

    // Αύξηση του jobID και της θέσης στην ουρά
    (*jobID)++;
    (*queuePosition)++;
}

// // Συνάρτηση για εισαγωγή μιας εργασίας στην ουρά αναμονής
// void issueJob(char* job, Queue* queued) {
//     static int jobID_counter = 1; // Μεταβλητή για τον αριθμό της εργασίας
//     char jobID[20]; // Μεταβλητή για το ID της εργασίας
    
//     // Δημιουργία του ID της εργασίας με τη μορφή "job_XX"
//     sprintf(jobID, "job_%02d", jobID_counter);
    
//     // Αύξηση του μετρητή του ID για την επόμενη εργασία
//     jobID_counter++;
    
//     // Προσθήκη της εργασίας στην ουρά αναμονής με την queuePosition αντίστοιχη με το μέγεθος της ουράς + 1
//     add(queued, jobID, job, queueSize(queued) + 1);
    
//     // Εκτύπωση της τριπλέτας <jobID, job, queuePosition> στον jobCommander
//     printf("JobID: %s, Job: %s, Queue Position: %d\n", jobID, job, queueSize(queued));
// }


// Συνάρτηση για τη διαχωρισμό μιας γραμμής εντολών σε ορίσματα
void parse(char *command, char **args) {
    char *token;
    int i = 0;

    // Χρήση της συνάρτησης strtok για τον διαχωρισμό των ορισμάτων
    token = strtok(command, " ");
    while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  // Τερματισμός πίνακα ορισμάτων με NULL
}

// Συνάρτηση που χειρίζεται το SIGCHLD σήμα
void handler(int sig) {
    pid_t pid;
    int status;

    // Χρήση της waitpid για την αναμονή του παιδιού
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Εδώ μπορείτε να πραγματοποιήσετε επιπλέον επεξεργασία εάν είναι απαραίτητο
        printf("Child process with PID %d terminated\n", pid);
    }
}


