#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdbool.h>
#include "../common.h"

typedef struct qNode *Queue;
struct qNode {
    char job[COMMANDS_BUFFER];
    int jobID;
    int clientSocket;
    Queue next;
};

void enqueue(Queue *queue, char *job, int jobID, int clientSocket);
Queue dequeue(Queue *queue);
void remove_job_from_queue(Queue *queue, int jobID);
bool isEmpty(Queue queue);
int get_first_job(Queue queue);
bool find_in_queue(Queue queue, int jobIdToStop);
void print_queue(Queue queue);
int counter(Queue queue);

#endif
