#include <stdbool.h>

// Δομή για τον κόμβο της ουράς αναμονής
typedef struct Node {
    char* jobID;  // Αναγνωριστικό της εργασίας
    char* job;    // Η εργασία
    int queuePosition; // Θέση στην ουρά
    int status;   // Κατάσταση της εργασίας (queued:0 ή running:1)
    struct Node* next; // Δείκτης προς τον επόμενο κόμβο
} Node;

// Δομή για την ουρά αναμονής
typedef struct {
    Node* front; // Δείκτης στον πρώτο κόμβο
    Node* rear;  // Δείκτης στον τελευταίο κόμβο
} Queue;


Queue* createQueue();
void add(Queue* queue, char* jobID, char* job, int queuePosition);
void remove_node(Queue* queue);
int isEmpty(Queue* queue);
void destroyQueue(Queue* queue);
void printQueue(Queue* queue);
int queueSize(Queue* queue);

bool is_running(Queue* running, int id);
bool is_queued(Queue* queued, int id);
bool stop_queued(Queue* queued, int id);
bool stop_running(Queue* running, int id);