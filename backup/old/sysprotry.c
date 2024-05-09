//job executor
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CONCURRENCY 4 // Μέγιστος βαθμός παραλληλίας

// Δομή για την αναπαράσταση μιας εργασίας
typedef struct {
    int id;
    // Εδώ θα μπορούσαμε να έχουμε περισσότερα πεδία που χαρακτηρίζουν την εργασία
} Job;

// Ουρά για τις εργασίες που περιμένουν να εκτελεστούν
Job *jobQueue;
int jobCount = 0;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;

// Σημαία που υποδεικνύει αν η εφαρμογή πρέπει να συνεχίσει να εκτελείται
int running = 1;

// Συνάρτηση που εκτελεί μια εργασία
void *executeJob(void *arg) {
    Job *job = (Job *)arg;
    printf("Executing job %d\n", job->id);
    sleep(2); // Υποκοριστική εργασία
    printf("Job %d finished\n", job->id);

    // Ελευθέρωση της μνήμης της εργασίας
    free(job);

    // Έλεγχος της ουράς για την έναρξη νέας εργασίας
    pthread_mutex_lock(&queueMutex);
    if (jobCount > 0) {
        Job nextJob = jobQueue[0];
        for (int i = 0; i < jobCount - 1; i++) {
            jobQueue[i] = jobQueue[i + 1];
        }
        jobCount--;
        pthread_mutex_unlock(&queueMutex);
        executeJob(&nextJob);
    } else {
        pthread_mutex_unlock(&queueMutex);
    }
    
    return NULL;
}

// Συνάρτηση για την υποβολή μιας εργασίας στην ουρά
void submitJob(Job job) {
    pthread_mutex_lock(&queueMutex);
    jobQueue[jobCount++] = job;
    pthread_mutex_unlock(&queueMutex);
}

// Κεντρική συνάρτηση του jobExecutor
void *jobExecutorMain(void *arg) {
    while (running) {
        // Έλεγχος του ρυθμού παραλληλίας και εκτέλεση των εργασιών
        pthread_mutex_lock(&queueMutex);
        int currentConcurrency = jobCount > MAX_CONCURRENCY ? MAX_CONCURRENCY : jobCount;
        pthread_mutex_unlock(&queueMutex);

        pthread_t threads[currentConcurrency];
        for (int i = 0; i < currentConcurrency; i++) {
            pthread_create(&threads[i], NULL, executeJob, &jobQueue[i]);
        }

        for (int i = 0; i < currentConcurrency; i++) {
            pthread_join(threads[i], NULL);
        }
    }
    return NULL;
}

int main() {
    // Αρχικοποίηση της ουράς εργασιών
    jobQueue = (Job *)malloc(MAX_CONCURRENCY * sizeof(Job));

    // Δημιουργία νημάτων για την εκτέλεση των εργασιών
    pthread_t executorThread;
    pthread_create(&executorThread, NULL, jobExecutorMain, NULL);

    // Προσομοίωση υποβολής εργασιών από το χρήστη
    for (int i = 0; i < 10; i++) {
        Job newJob;
        newJob.id = i + 1;
        submitJob(newJob);
        sleep(1);
    }

    // Αναμονή για την ολοκλήρωση του jobExecutor
    running = 0;
    pthread_join(executorThread, NULL);

    // Απελευθέρωση της μνήμης της ουράς εργασιών
    free(jobQueue);

    return 0;
}
