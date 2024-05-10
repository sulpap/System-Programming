#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include "defines.h"

// adds a job to the queue
// TODO: rename to +enqueue+
void enqueue(Queue *queue, char *job, int jobID) {
    // allocate the memory needed
    Queue newNode = (Queue)malloc(sizeof(struct qNode));
    if (newNode == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // assign the values
    strcpy(newNode->job, job);
    newNode->jobID = jobID;
    newNode->next = NULL;

    // add it to the queue
    if (*queue == NULL) { // if it's empty, just add it
        *queue = newNode;
    } else { // else, add it to the end
        Queue current = *queue;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

// delete job from list , if it exists
Queue dequeue(Queue *queue) {
    if (isEmpty(*queue)){
        return NULL;
    }

    Queue result = *queue;
    *queue = (*queue)->next;

    return result;
}

// delete certain job from list , if it exists
void remove_job(Queue *queue, int jobID) {
    Queue current_queue = *queue;
    Queue prev_queue = NULL;

    while (current_queue != NULL) {
        if (current_queue->jobID == jobID) {
            if (prev_queue == NULL) {
                *queue = current_queue->next;
            } else {
                prev_queue->next = current_queue->next;
            }
            free(current_queue);
            return;
        }
        prev_queue = current_queue;
        current_queue = current_queue->next;
    }

    printf("Job with ID %d not found in the queue.\n", jobID);
}

// true: queue is empty, false: it's not
bool isEmpty(Queue queue) {
    return queue == NULL;
}

// returns jobID of first job in the queue --> oldest
int get_first_job(Queue queue){
    if (!isEmpty(queue)) {
        return queue->jobID;
    } else {
        return -1;
    }
}

// prints all jobs in the queue

void print_queue(Queue queue) {
    if (isEmpty(queue)) {
        printf("%s Queue is empty.\n", LOG_PREFIX);
        return;
    }
    Queue current = queue;
    int queuePosition = 0;
    while (current != NULL) {
        printf("%s QUEUE: <job_%d, %s, %d>\n", LOG_PREFIX, current->jobID, current->job, queuePosition);
        queuePosition++;
        current = current->next;
    }
    return;
}

void get_print_queue(Queue queue, char **arr) {
    if (isEmpty(queue)) {
        printf("Queue is empty.\n");
        return;
    }
    Queue current = queue;
    int i = 0;
	char buf[500];

    while (current != NULL) {
        sprintf(buf, "job_%d,%s,%d", queue->jobID, queue->job, i);
        //printf("QUEUE: Job ID: %d, Job: %s\n", current->jobID, current->job);
        arr[i] = malloc(sizeof(char)*(strlen(buf)+1));
        strcpy(arr[i], buf);
        i++;
        current = current->next;
    }
    arr[i] = NULL;
    
    return;
}



// counter for jobs in the queue
int counter(Queue queue) {
    int count = 0;
    Queue current = queue;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}
