#include "list.h"
#include <stdlib.h>
#include <stdio.h>

// adds a job to the queue
void add(Queue *queue, char *job, int jobID) {
    // allocate the memory needed
    Queue newNode = (Queue)malloc(sizeof(struct qNode));
    if (newNode == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // assign the values
    newNode->job = job;
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
        printf("Queue is empty.\n");
        return;
    }
    Queue current = queue;
    while (current != NULL) {
        printf("Job ID: %d, Job: %s\n", current->jobID, current->job);
        current = current->next;
    }
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

int main() {
    Queue queue = NULL; // Initialize an empty queue

    // Add some jobs to the queue
    for (int i=0; i<16; i++){ //15 jobs
        add(&queue, "Job", i);
    }

    // Print the queue
    printf("Initial queue:\n");
    print_queue(queue);

    // Remove a job by ID
    remove_job(&queue, 2);
    printf("\nQueue after removing Job 2:\n");
    print_queue(queue);

    // Get the job ID of the first job in the queue
    if (!isEmpty(queue)) {
        int firstJobID = get_first_job(queue);
        printf("\nJob ID of the first job in the queue: %d\n", firstJobID);
    } else {
        printf("\nQueue is empty.\n");
    }

    // Get the number of jobs in the queue
    printf("\nNumber of jobs in the queue: %d\n", counter(queue));

    return 0;
}
