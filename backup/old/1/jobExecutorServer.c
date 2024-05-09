#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>

#include "myheaders.h"
#include "list.h"

Queue* running = NULL;
Queue* queued = NULL;
int Concurrency;

void exit_do(int fd);
void update_running_jobs();
// bool is_running(Queue* running, int id);
// bool is_queued(Queue* queued, int id);
// bool stop_queued(Queue* queued, int id);
// bool stop_running(Queue* running, int id);

void jobExecutorServer() {
    int fd1, jobID = 0, id;
    char buf[SIZE + 1], response[SIZE];
    char *operation, *parameter;
    bool flag; // true: we know the command, false: we don't

    // we make the pipe to send back response to commander
    if( mkfifo(PIPE2, 0666) < 0 ) {
        if (errno != EEXIST ) {
            perror ("mkfifo (PIPE2) failed");
            exit(1);
        }
    }

    // running = createQueue();        // is this needed????
    // queued = createQueue();

    while (1) {
        // we open the pipe for reading/writing
        fd1 = open(PIPE1, O_RDWR);
        if (fd1 == -1) {
            perror("Server reading - pipe open error");
            exit(1);
        }

        // we read the command from the pipe
        if( read(fd1, buf, sizeof(buf)) == -1) { // if (read(fd1, buf, SIZE+1) == -1) {
            perror("Server reading - read error");
            exit(1);
        }

        // closing reading pipe
        close(fd1);

        // Διαχωρισμός του μηνύματος σε λειτουργία και παραμέτρους
        operation = strtok(buf, " ");
        parameter = strtok(NULL, "");

        // Ανάλυση της λειτουργίας και εκτέλεση αντίστοιχης ενέργειας
        if (strcmp(operation, "issueJob") == 0) {
            flag = true;
            if (parameter == NULL) {
                printf("Missing argument for issueJob\n");
            } else {
                issueJob(parameter, &running, &queued, jobID, 1);
                jobID++;
                // id++;
            }
            // break;
        }
            
        if (strcmp(operation, "setConcurrency") == 0) {
            flag = true;
            Concurrency = atoi(parameter); // convert string to int in order to use
            sprintf(response, "%s %d", "Concurrency changed to :", Concurrency); //print & store string
            //respond
            respond_string(response); // απάντηση στον client με το νέο concurrency
            //update running queue
            update_running_jobs();
        }
        
        if (strcmp(operation, "stop") == 0) {
            flag = true;
            int id = atoi(parameter);
            // if the job is not running
            //if (!is_running(running, id)) {
                //if not queued -> print not found
                // if (!is_queued(queued, id)) { /////////////////////////////////////
                //     printf("Job with ID %d not found.\n", id); 
                // } else {
                    // if queued -> stop it
                //     stop_queued(queued, id);
                //     printf("job_%02d removed\n", id);
                // }
            // } else {
            //     // else - job is runnning -> stop it
            //     stop_running(running, id);
            //     printf("job_%02d terminated\n", id);
            // }
        }

        if (strcmp(operation, "poll") == 0) {
            flag = true;
            if (strcmp(parameter, "running") == 0) {
                // Εκτύπωση της τριπλέτας για κάθε εργασία που τρέχει
                printQueue(running);
                // Δεν επιστρέφεται μήνυμα στον jobCommander
            } else if (strcmp(parameter, "queued") == 0) {
                // Εκτύπωση της τριπλέτας για κάθε εργασία που είναι στην ουρά αναμονής
                printQueue(queued);
                // Δεν επιστρέφεται μήνυμα στον jobCommander
            }
        }
            // if the job is running -> 
                // print triplet (from running  list)
                // respond string
            // else if job is queued -> 
                // print triplet (from queued list)
                // respond strings

            // free args ????????/
        

        if (strcmp(operation, "exit") == 0) {
            flag = true;
            // unlink(jobExecutorServer_file);
            // sprintf(response, "%s", "Server is exiting...");
            // respond(response);
            // exit(0);
            exit_do(fd1);
        }

        fflush(stdout); 

        /* stack pverflow: A fflush(stdout) checks if there 
        are any data in the buffer that should be written and 
        if so, the underlying syscall is used to write the data to 
        the OS. */

        if (flag == false) {
            printf("This command: %s is unknown", response);
            // respond using response
            respond_string(response);
        }
        fflush(stdout);
    }
}

// function to respond to commander --> string
void respond_string(char *args) {
    int fd;
    char buf[SIZE + 1];
    
    // Άνοιγμα του pipe για εγγραφή
    fd = open(PIPE2, O_WRONLY);
    if ( fd == -1) {
        perror("Server writing - pipe open error");
        exit(1);
    }

    // Αντιγραφή ορισμάτων στο buffer
    strcpy(buf, args);

    // Εγγραφή στο pipe
    if (write(fd, buf, SIZE + 1) == -1) {
        perror("Server writing - write error");
        exit(1);
    }

    // Καθαρισμός buffer
    memset(buf, 0, SIZE + 1);

    // Εγγραφή "exit" στο PIPE
    strcpy(buf, "exit");
    if (write(fd, buf, SIZE + 1) == -1) {
        perror("Server writing - write error");
        exit(1);
    }

    // Κλείσιμο pipe
    close(fd);
}

// function to respond to commander --> array of strings
void respond_array(char **args) {
    int fd;
    char buf[SIZE + 1];

    // Άνοιγμα pipe για εγγραφή
    fd = open(PIPE2, O_WRONLY);
    if ( fd == -1) {
        perror("Server writing - pipe open error");
        exit(1);
    }

    // Εγγραφή απάντησης στο pipe
    for (int i = 0; args[i] != NULL; i++) {
        // Αντιγραφή του κάθε στοιχείου στον buffer
        strcpy(buf, args[i]);
        
        // Εγγραφή στο PIPE2
        if (write(fd, buf, SIZE + 1) == -1) {
            perror("Server writing - write error");
            exit(1);
        }
    }

    // Εγγραφή "exit" στο pipe
    memset(buf, 0, SIZE + 1); // Καθαρισμός buffer
    strcpy(buf, "exit"); // Αντιγραφή της λέξης "exit"
    if (write(fd, buf, SIZE + 1) == -1) {
        perror("Server writing - write error");
        exit(1);
    }

    // Κλείσιμο pipe
    close(fd);
}

void exit_do(int fd) {
    char exit_msg[] = "Server is exiting...";

    // Διαγραφή του pipe
    if ( unlink(jobExecutorServer_file) == -1) {
        perror("unlink failed");
        exit(1);
    }

    // Αποστολή μηνύματος εξόδου στον client
    respond_string(exit_msg);

    // Κλείσιμο του pipe προς αποφυγή διαρροής πόρων
    close(fd);

    // Τερματισμός του server
    exit(0);
    return;
}


void update_running_jobs() {
    int running_jobs = queueSize(running);

    if (running_jobs < Concurrency) { // //If I am permitted to execute
        int i;
        int remaining_capacity = Concurrency - running_jobs; // Υπολογισμός του ανεκτέλεστου χώρου στον πίνακα των τρεχουσών εργασιών

        // for (i = 0; i < remaining_capacity; i++) {
        //     if (!isEmpty(queued)) {
        //         // Αν υπάρχουν εργασίες στην ουρά αναμονής, μεταφέρουμε τις πρώτες (concurrency - running) εργασίες στη λίστα των τρεχουσών εργασιών
        //         Node* job_node = queued->front;
        //         char* jobID = job_node->jobID;
        //         char* job = job_node->job;
        //         // strcpy(job, job_node->job);
        //         int queuePosition = job_node->queuePosition;
                
        //         // Αφαίρεση της εργασίας από την ουρά αναμονής
        //         delete(queued);

        //         // Εισαγωγή της εργασίας στη λίστα των τρεχουσών εργασιών
        //         add(running, jobID, job, queuePosition); // ????

        //         // Εκτέλεση της εργασίας
        //         issueJob(job, running, queued, atoi(jobID), 0);
        //     } else {
        //         // Αν η ουρά αναμονής είναι άδεια, δεν υπάρχουν περισσότερες εργασίες προς εκτέλεση
        //         break;
        //     }
        // }
        // else, if due to small Concurrency i can't run the command add it to pending queue to
		// be executed later ?????????????
    }
}

// // Συνάρτηση για να ελέγξει αν μια εργασία τρέχει
// bool is_running(Queue running, int id) {
//     Node* current = running;
//     while (current != NULL) {
//         if (current->jobID == id && current->status == 1) {
//             return true;
//         }
//         current = current->next;
//     }
//     return false;
// }
// /*Αυτή η συνάρτηση ελέγχει αν ένα job με το συγκεκριμένο id τρέχει αυτή τη στιγμή. Ξεκινά από τον πρώτο κόμβο της running ουράς και ελέγχει αν το jobID του κόμβου είναι ίσο με το δοσμένο id και αν η κατάσταση της εργασίας είναι 1 (που σημαίνει ότι τρέχει). Αν βρεθεί μια τέτοια εργασία, η συνάρτηση επιστρέφει true, διαφορετικά επιστρέφει false.*/

// // Συνάρτηση για να ελέγξει αν μια εργασία είναι στην ουρά αναμονής
// bool is_queued(Queue* queued, int id) {
//     Node* current = queued->front;
//     while (current != NULL) {
//         if (atoi(current->jobID) == id && current->status == 0) {
//             return true;
//         }
//         current = current->next;
//     }
//     return false;
// }
// /*Αυτή η συνάρτηση ελέγχει αν ένα job με το δοσμένο id είναι στην ουρά αναμονής (queued). Ξεκινά από τον πρώτο κόμβο της ουράς αναμονής και ελέγχει αν το jobID του κόμβου είναι ίσο με το δοσμένο id και αν η κατάσταση της εργασίας είναι 0 (που σημαίνει ότι είναι στην ουρά αναμονής). Αν βρεθεί μια τέτοια εργασία, η συνάρτηση επιστρέφει true, διαφορετικά επιστρέφει false.*/

// // Συνάρτηση για να σταματήσει μια εργασία που είναι στην ουρά αναμονής
// bool stop_queued(Queue* queued, int id) {
//     Node* current = queued->front;
//     while (current != NULL) {
//         if (atoi(current->jobID) == id && current->status == 0) {
//             current->status = -1; // Σταματάει την εργασία στην ουρά αναμονής
//             return true;
//         }
//         current = current->next;
//     }
//     return false;
// }

// // Συνάρτηση για να σταματήσει μια εργασία που τρέχει
// bool stop_running(Queue* running, int id) {
//     Node* current = running->front;
//     while (current != NULL) {
//         if (atoi(current->jobID) == id && current->status == 1) {
//             current->status = -1; // Σταματάει την εργασία που τρέχει
//             kill(current->job, SIGKILL); //////// ????????????
//             return true;
//         }
//         current = current->next;
//     }
//     return false;
// }
