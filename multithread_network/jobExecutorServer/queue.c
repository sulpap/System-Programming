#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include "defines.h"

// adds a job to the queue
void enqueue(Queue *queue, char *job, int jobID, int clientSocket) {
    // allocate the memory needed
    Queue newNode = (Queue)malloc(sizeof(struct qNode));
    if (newNode == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // assign the values
    strcpy(newNode->job, job);
    newNode->jobID = jobID;
    newNode->clientSocket = clientSocket;
    newNode->isActive = false;
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

// delete oldest job from queue , if it exists, and return it
Queue dequeue(Queue *queue) {
    if (isEmpty(*queue)){
        return NULL;
    }

    Queue result = *queue;
    *queue = (*queue)->next;

    return result;
}

// delete certain job from queue , if it exists
void remove_job_from_queue(Queue *queue, int jobID) {
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

    printf("%s: Job with ID %d not found in the queue.\n", LOG_PREFIX, jobID);
}

// true: queue is empty, false: it's not
bool isEmpty(Queue queue) {
    return queue == NULL;
}

// if the job is in the queue, return true. if it's not return false.
bool find_in_queue(Queue queue, int jobIdToStop) {
    bool found = false;
    while (!found && queue != NULL) {
        if (queue->jobID == jobIdToStop) {
            found = true;
        }
    }
    return found;
}

// prints all jobs in the queue
void print_queue(Queue queue) {
    if (isEmpty(queue)) {
        printf("%s: Queue is empty.\n", LOG_PREFIX);
        return;
    }
    Queue current = queue;
    while (current != NULL) {
        printf("%s: QUEUE: <job_%d, %s, %d>\n", LOG_PREFIX, current->jobID, current->job, current->clientSocket);
        current = current->next;
    }
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

// for every job in the queue, respond to the respective client
void notify_clients(Queue queue)
{ 
  Queue job = queue;
  while (job != NULL) {
    respond_to_commander(job->clientSocket, "JOB TERMINATED BEFORE EXECUTION");
    job = job->next;
  }
}