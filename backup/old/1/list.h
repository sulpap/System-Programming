#include <stdbool.h>

typedef struct qNode *Queue;
struct qNode {
    char *job;
    int jobID;
    Queue next;
};

// Συναρτήσεις για την διαχείριση της ουράς
void add(Queue *queue, char *job, int jobID);
void delete_job(Queue *, int);
bool isEmpty(Queue queue);
get_first_job(Queue queue);
void printQueue(Queue queue);
int queueSize(Queue queue);