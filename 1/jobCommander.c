// jobCommander
#include <signal.h>

#include "myheaders.h"

// extern char *PIPE1;
// extern char *PIPE2;
// extern int SIZE;

void jobCommander(char **msg, int server_pid) {
    int fd1, fd2;
    char buf[SIZE+1];
    //char buf2[SIZE+1];

    // we open the pipe to read data
    fd1 = open(PIPE1, O_WRONLY);
    if (fd1 == -1) {
        perror("Commander writing: pipe open error");
        return;
    }

   /*Συνολικά, αυτό το κομμάτι κώδικα συγκεντρώνει όλες τις παραμέτρους της εντολής 
    σε ένα μήνυμα, ξεκινώντας από το msg[1] και προσθέτοντας τις επιπλέον παραμέτρους 
    μέχρι να φτάσει στο τέλος της λίστας των παραμέτρων.*/
   
    int i = 1; // Ξεκινάμε από το δεύτερο στοιχείο του πίνακα msg
    // Αντιγράφουμε την πρώτη παράμετρο
    sprintf(buf, "%s", msg[1]);
    // Συνεχίζουμε να προσθέτουμε τις υπόλοιπες παραμέτρους στο buf
    while (msg[i] != NULL) {
        sprintf(buf, "%s %s", buf, msg[i]);
        i++;
    }

    // we write the string in the pipe
    if( (write(fd1,buf,SIZE+1)) == -1 ){
        perror("Commander writing: write error");
        return 1;
    }

    close(fd1);

    // we open the second pipe to receive and print the response
    fd2 = open(PIPE2, O_RDWR);
    if (fd2 == -1) {
        perror("Commander reading: pipe open error");
        return;
    }

    //while server is sending something back
        //read response
		//if I read "exit" this means the end of arguments
		//print response

    while (1) {
        ssize_t bytes_read = read(fd2, buf, SIZE); // Διαβάζουμε από το pipe

        if (bytes_read == -1) {
            perror("Commander reading: read error");
            break; // Έξοδος από τον βρόχο σε περίπτωση σφάλματος
        } else if (bytes_read == 0) {
            // Το pipe έκλεισε, τερματίζουμε τον βρόχο
            break;
        } else {
            // Εκτυπώνουμε την απάντηση που λάβαμε
            buf[bytes_read] = '\0'; // Τερματίζουμε το string με NULL χαρακτήρα
            printf("Response from server: %s\n", buf);

            // Έλεγχος για το "exit" για τερματισμό του βρόχου
            if (strcmp(buf, "exit") == 0) {
                break;
            }
        }
    }

    close(fd2);

}