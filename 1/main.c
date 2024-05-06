#include <signal.h>

#include "myheaders.h"

char *PIPE1 = "myfifo1";
char *PIPE2 = "myfifo2";
int SIZE = 1024;
char *jobExecutorServer_file = "jobExecutorServer_file";   // jobExecutorServer_file.txt ??????????????????

int main(int argc, char *argv[])
{
    if (argc == 1) {
		printf("Wrong number of arguments\n");
		exit(1);
	}

    FILE *server_file;
    int pid;

    // we check if the file exists
    server_file = fopen(jobExecutorServer_file, "r");

    // if it doesn't, we create the server
    if(server_file == NULL) {
        pid_t server_pid = fork();
		if (server_pid < 0) {
			perror("Error while creating jobExecutorServer -- Fork failed");
			exit(1);
		}

        if (server_pid == 0)    //child
        {
            //server : write pid to file so that jobCommander knows if server is active
			server_file = fopen(jobExecutorServer_file, "w");
			fprintf(server_file, "%d\n", getpid());
			fclose(server_file);
            
            /* or: 
            if ((fd = open(server_file, O_RDWR | O_CREAT, 0644)) == -1) //create serverinfo file containing pid
	    	{
			perror("Error while creating serverinfo file: ");
			exit(1);
		    }*/

			jobExecutorServer();
            // Όταν η διεργασία του jobExecutorServer ξεκινά, μπορεί να γράψει το δικό της PID (Process ID) 
            //σε ένα αρχείο. Αυτό το αρχείο μπορεί να χρησιμοποιηθεί από τον jobCommander για να ελέγξει αν 
            //ο jobExecutorServer είναι ενεργός ή όχι, ανάλογα με την ύπαρξη ή όχι του αρχείου και το περιεχόμενό του. 
            //Κατά τη διαδικασία τερματισμού του jobExecutorServer, αυτός μπορεί να διαγράψει το αρχείο για να 
            //ειδοποιήσει τον jobCommander ότι έχει τερματιστεί.
        }
        else // parent (--> we create it here)
        {
			pid = server_pid;
            // we create the server pipe
			if (mkfifo (PIPE1, 0666) < 0){
				if (errno != EEXIST ) {
					perror ("mkfifo failed") ;
					exit(1);
				}
			}
		}
    }
    else // if it exists, we just copy its pid
    {
        fscanf(server_file, "%d", &pid);
		fclose(server_file);
    }

    jobCommander(argv, pid);

    //mkfifo(PIPE1, 0666);
    //mkfifo(PIPE2, 0666);
}




//  // Έλεγχος αν το αρχείο jobExecutorServer.txt υπάρχει
//     if (access("jobExecutorServer.txt", F_OK) != -1) {
//         // Το αρχείο υπάρχει, άρα ο jobExecutorServer είναι ενεργός
//         printf("Ο jobExecutorServer είναι ενεργός.\n");
//     } else {
//         // Το αρχείο δεν υπάρχει, άρα ο jobExecutorServer δεν είναι ενεργός
//         printf("Ο jobExecutorServer δεν είναι ενεργός. Πρέπει να τον εκκινήσετε.\n");
//     }