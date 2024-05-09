#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_JOB_NAME_LEN 100

// Structure for a job node
typedef struct JobNode {
    int jobID;
    int pid;  // For running jobs only, set to -1 for queued jobs
    char job[MAX_JOB_NAME_LEN];
    struct JobNode* next;
} JobNode;

// Add a job to the list (either running or queued)
void addToJobList(JobNode** job_list, int jobID, const char* job, int pid) {
    JobNode* new_node = (JobNode*)malloc(sizeof(JobNode));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    new_node->jobID = jobID;
    new_node->pid = pid;
    strncpy(new_node->job, job, MAX_JOB_NAME_LEN - 1);
    new_node->job[MAX_JOB_NAME_LEN - 1] = '\0';
    new_node->next = *job_list;
    *job_list = new_node;
}

// Print the jobs list (either running or queued)
void printJobList(JobNode* job_list, const char* title) {
    printf("%s:\n", title);
    while (job_list != NULL) {
        printf("JobID: %d, PID: %d, Job: %s\n", job_list->jobID, job_list->pid, job_list->job);
        job_list = job_list->next;
    }
}

// Remove a job from the list (either running or queued)
void removeFromJobList(JobNode** job_list, int jobID) {
    JobNode* current = *job_list;
    JobNode* prev = NULL;

    while (current != NULL && current->jobID != jobID) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        fprintf(stderr, "Job with ID %d not found\n", jobID);
        return;
    }

    if (prev == NULL) {
        // Job to be removed is the first node
        *job_list = current->next;
    } else {
        prev->next = current->next;
    }

    free(current);
}
