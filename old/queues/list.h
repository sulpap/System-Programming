#include <stdbool.h>

typedef struct qNode *Queue;
struct qNode {
    char *job;
    int jobID;
    Queue next;
};

// Συναρτήσεις για την διαχείριση της ουράς
void add(Queue *queue, char *job, int jobID);
void remove_job(Queue *, int);
bool isEmpty(Queue queue);
int get_first_job(Queue queue);
void print_queue(Queue queue);
int counter(Queue queue);