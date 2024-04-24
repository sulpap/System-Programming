// Δομή για τον κόμβο της ουράς αναμονής
typedef struct Node {
    char* jobID;  // Αναγνωριστικό της εργασίας
    char* job;    // Η εργασία
    int queuePosition; // Θέση στην ουρά
    struct Node* next; // Δείκτης προς τον επόμενο κόμβο
} Node;

// Δομή για την ουρά αναμονής
typedef struct {
    Node* front; // Δείκτης στον πρώτο κόμβο
    Node* rear;  // Δείκτης στον τελευταίο κόμβο
} Queue;


Queue* createQueue();
void add(Queue* queue, char* jobID, char* job, int queuePosition);
void remove(Queue* queue);
int isEmpty(Queue* queue);
void destroyQueue(Queue* queue);
void printQueue(Queue* queue);