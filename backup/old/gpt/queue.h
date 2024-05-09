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