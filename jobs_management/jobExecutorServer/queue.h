#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdbool.h>
#include "../common.h"

typedef struct qNode *Queue;
struct qNode {
    char job[COMMANDS_BUFFER];
    int jobID;
    Queue next;
};

void enqueue(Queue *queue, char *job, int jobID);
Queue dequeue(Queue *queue);
void remove_job(Queue *queue, int jobID);
bool isEmpty(Queue queue);
int get_first_job(Queue queue);
void print_queue(Queue queue);
void get_print_queue(Queue queue, char **arr);
int counter(Queue queue);

#endif
