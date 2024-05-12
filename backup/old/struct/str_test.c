#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

// Define your struct
typedef struct runningNode {
    pid_t pid;
    int jobId;
    int queuePosition;
    char command[10];
} RunningNode;

RunningNode *running = NULL; // Global running array
int numberOfRunningJobs = 0;

// Define the maximum number of running jobs
#define MAX_RUNNING_JOBS 100

// Function to print the contents of the array
void printRunningArray() {
    printf("Contents of the running array:\n");
    for (int i = 0; i < numberOfRunningJobs; i++) {
        printf("Index %d:\n", i);
        printf("  pid: %d\n", running[i].pid);
        printf("  jobId: %d\n", running[i].jobId);
        printf("  queuePosition: %d\n", running[i].queuePosition);
        printf("  command: %s\n", running[i].command);
    }
}

// Function to add a new element to the array
RunningNode *addRunningNode(pid_t pid, int jobId, int queuePosition, const char *command) {
    // Check if the array is already full
    if (numberOfRunningJobs >= MAX_RUNNING_JOBS) {
        // You may want to handle the case of array being full
        fprintf(stderr, "Array is already full\n");
        return running;
    }

    // Reallocate memory to expand the array
    running = (RunningNode *)realloc(running, numberOfRunningJobs * sizeof(RunningNode));
    if (running == NULL) {
        // Handle reallocation failure
        fprintf(stderr, "Memory reallocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Add the new element at the end of the array
    running[numberOfRunningJobs-1].pid = pid;
    running[numberOfRunningJobs-1].jobId = jobId;
    running[numberOfRunningJobs-1].queuePosition = queuePosition;
    // Copy the command string to the command array
    snprintf(running[numberOfRunningJobs-1].command, sizeof(running[numberOfRunningJobs-1].command), "%s", command);

    return running;
}

bool deleteRunningNodeByPid(pid_t pid) {
    int index = -1;

    // Find the index of the element with the given pid
    for (int i = 0; i < numberOfRunningJobs; i++) {
        if (running[i].pid == pid) {
            index = i;
            break;
        }
    }

    // If element with given pid is found
    if (index != -1) {
        // Calculate the number of bytes to move
        size_t bytesToMove = (numberOfRunningJobs - index - 1) * sizeof(RunningNode);

        // Shift elements to the left to overwrite the element to be deleted
        memmove(&running[index], &running[index + 1], bytesToMove);

        // Decrease the number of running jobs
        numberOfRunningJobs--;

        // Reallocate memory to shrink the array
        running = (RunningNode *)realloc(running, numberOfRunningJobs * sizeof(RunningNode));
        if (running == NULL && numberOfRunningJobs > 0) {
            // Handle reallocation failure
            fprintf(stderr, "Memory reallocation failed\n");
            exit(EXIT_FAILURE);
        }
        return true; // Element found and deleted
    }
    return false; // Element not found
}


int main() {
    // Add a new element to the array
    numberOfRunningJobs++;
    addRunningNode(1234, 1, 1, "example_command");
    
    numberOfRunningJobs++;
    addRunningNode(2345, 1, 1, "example_command2");

    numberOfRunningJobs++;
    addRunningNode(345, 1, 1, "example_command3");

    numberOfRunningJobs++;
    addRunningNode(45, 1, 1, "example_command4");

    // Print the contents of the array
    printRunningArray();

    // Delete an element from the array
    deleteRunningNodeByPid(1234);

    deleteRunningNodeByPid(45);

    // Print the contents of the array after deletion
    printRunningArray();

    // Free the memory allocated for the array
    free(running);

    return 0;
}
