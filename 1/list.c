#include "includes.h"
#include "list.h"

// Συνάρτηση για τη δημιουργία μιας νέας ουράς
Queue* createQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL;
    return queue;
}

// Συνάρτηση για να εισάγει μια νέα εργασία στην ουρά
void add(Queue* queue, char* jobID, char* job, int queuePosition) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    newNode->jobID = jobID;
    newNode->job = job;
    newNode->queuePosition = queuePosition;
    newNode->next = NULL;

    // Εισαγωγή στην ουρά
    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

// Συνάρτηση για να αφαιρέσει μια εργασία από την αρχή της ουράς
void remove(Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty. Nothing to dequeue.\n");
        return;
    }

    Node* temp = queue->front;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(temp);
}

// Συνάρτηση για να ελέγξει αν η ουρά είναι κενή
int isEmpty(Queue* queue) {
    return queue->front == NULL;
}

// Συνάρτηση για να απελευθερώσει τη μνήμη που δεσμεύθηκε για την ουρά
void destroyQueue(Queue* queue) {
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}

// Συνάρτηση για εκτύπωση της ουράς
void printQueue(Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty.\n");
        return;
    }
    Node* current = queue->front;
    while (current != NULL) {
        printf("Job ID: %s, Job: %s, Queue Position: %d\n", current->jobID, current->job, current->queuePosition);
        current = current->next;
    }
}





// int main() {
//     // Δημιουργία μιας νέας ουράς
//     Queue* myQueue = createQueue();

//     // Εισαγωγή εργασιών στην ουρά
//     enqueue(myQueue, "job_1", "ls -l /path/to/directory1", 1);
//     enqueue(myQueue, "job_2", "wget aUrl", 2);
//     enqueue(myQueue, "job_3", "grep 'keyword' /path/to/file1", 3);

//     // Εκτύπωση της ουράς
//     printf("Initial Queue:\n");
//     printQueue(myQueue);

//     // Αφαίρεση μιας εργασίας από την ουρά
//     dequeue(myQueue);

//     // Εκτύπωση της ουράς μετά την αφαίρεση
//     printf("\nQueue after dequeue:\n");
//     printQueue(myQueue);

//     // Αποδέσμευση μνήμης
//     destroyQueue(myQueue);

//     return 0;
// }
