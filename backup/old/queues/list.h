#include <stdbool.h>

typedef struct qNode *Queue;
struct qNode {
    char *job;
    int jobID;
    Queue next;
};

void add(Queue *queue, char *job, int jobID);
Queue dequeue(Queue *queue);
void remove_job(Queue *queue, int jobID);
bool isEmpty(Queue queue);
int get_first_job(Queue queue);
void print_queue(Queue queue, char **arr);
int counter(Queue queue);