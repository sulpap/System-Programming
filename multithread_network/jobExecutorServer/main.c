#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "respond_to_commander.h"
#include "../common.h"
#include "defines.h"
#include "utility.h"
#include "queue.h"
#include "execute_command.h"

Queue queue = NULL;

int concurrencyLevel = 1; // default
int numberOfRunningJobs = 0;
int jobId = 0;
int bufferSize = 0;
bool server_running = true;

pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t concurrency_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t running_jobs_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t job_cond = PTHREAD_COND_INITIALIZER; // for controlling flow of jobs

void create_child_process(Queue *queue, int clientSocket);

void* worker() 
{
  while(1)
  {  
    pthread_mutex_lock(&queue_mutex);
    
    printf("%s worker: %ld is sleeping\n", LOG_PREFIX, pthread_self());
    
    while (server_running && queue == NULL) {
      pthread_cond_wait(&queue_cond, &queue_mutex); // wait for a job to be enqueued
      printf("%s worker: %ld is awake\n", LOG_PREFIX, pthread_self());
    }

    if (!server_running) {
      pthread_mutex_unlock(&queue_mutex);
      pthread_exit(NULL);
    }

    Queue job = queue;
    while (job != NULL && job->activeJob) {
      job = job->next;
    }

    if (job == NULL) {
      pthread_mutex_unlock(&queue_mutex);
      continue;
    }

    job->activeJob = true;
    
    printf("%s Worker %ld writing: processing job!\n", LOG_PREFIX, pthread_self());

    pthread_mutex_lock(&concurrency_mutex);

    // while (numberOfRunningJobs >= concurrencyLevel) {
    if (numberOfRunningJobs >= concurrencyLevel) {
      pthread_cond_wait(&job_cond, &concurrency_mutex); // wait for available slots
    }

    pthread_mutex_unlock(&concurrency_mutex);

    create_child_process(&queue, queue->clientSocket);

    // queue mutex is unlocked inside create_child_process.

  }
  pthread_exit(NULL);
}

void create_child_process(Queue *queue, int clientSocket)
{
  Queue job = dequeue(queue); // removing job from queue to be executed
  pthread_mutex_unlock(&queue_mutex);

  if (job == NULL)
  {
    printf("%s No job in the queue to run.\n", LOG_PREFIX);
    free(job);
    return;
  }

  printf("%s Job got out of the queue ready to run %s\n", LOG_PREFIX, job->job);

  pthread_mutex_lock(&running_jobs_mutex);
  numberOfRunningJobs++;
  pthread_mutex_unlock(&running_jobs_mutex);

  // fork one child process to execute the command
  pid_t pid = fork();
  if (pid == 0)
  {
    // child process
    // create output file
    char output_filename[20];
    sprintf(output_filename, "%d.output", getpid());

    FILE *output_file = fopen(output_filename, "w");
    
    if (output_file == NULL)
    {
      perror("Error creating output file");
      exit(EXIT_FAILURE);
    }
    
    // redirect stdout to output file
    dup2(fileno(output_file), STDOUT_FILENO);
    close(fileno(output_file));

    // execute the command
    execute_command(job->job);

    exit(EXIT_SUCCESS);
  }
  else if (pid > 0)
  {
    int status;
    waitpid(pid, &status, 0);
    printf("%s: Child process with pid %d finished.\n", LOG_PREFIX, pid);

    // Read output file and send its contents to the client

    char output_filename[20];
    sprintf(output_filename, "%d.output", pid);

    FILE *output_file = fopen(output_filename, "r");
    if (output_file == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    char output_buffer[COMMANDS_BUFFER];
    memset(output_buffer, 0, sizeof(output_buffer));
    //char responses_buffer[COMMANDS_BUFFER];

    size_t bytes_read;
    while ((bytes_read = fread(output_buffer, 1, sizeof(output_buffer), output_file)) > 0) {
      continue;
    }

    //sprintf(responses_buffer, "-----jobID output start-----\n\n%s\n-----jobID output end-----\n", output_buffer);
    respond_to_commander(clientSocket, "-----jobID output start-----\n\n");
    respond_to_commander(clientSocket, output_buffer);
    respond_to_commander(clientSocket, "\n-----jobID output end-----\n");

    // Close clientSocket
    close(clientSocket);

    // Remove the output file
    if (remove(output_filename) != 0) {
      perror("Error removing output file");
    }

    pthread_mutex_lock(&running_jobs_mutex);
    numberOfRunningJobs--;
    pthread_mutex_unlock(&running_jobs_mutex);
    pthread_cond_signal(&queue_cond);
    pthread_cond_signal(&job_cond);
  }
  else
  {
    perror("fork failed");
  }
  free(job);
}

void handle_issue_job(char *input_buffer, char *responses_buffer, int clientSocket) 
{
  jobId++;

  // remove the command (issueJob, in this case)
  remove_first_word(input_buffer);

  // put the job in the queue if there is space
  pthread_mutex_lock(&queue_mutex);

  if (counter(queue) >= bufferSize) 
  {
    printf("%s Buffer is full, waiting for an empty slot...\n", LOG_PREFIX);
    pthread_cond_wait(&queue_cond, &queue_mutex);
  }

  if (counter(queue) < bufferSize)
  {
    enqueue(&queue, input_buffer, jobId, clientSocket);

    pthread_mutex_lock(&concurrency_mutex);
    pthread_mutex_lock(&running_jobs_mutex);
    if (numberOfRunningJobs < concurrencyLevel)
    {
      pthread_cond_signal(&queue_cond); // notify waiting threads
    }
    pthread_mutex_unlock(&running_jobs_mutex);
    pthread_mutex_unlock(&concurrency_mutex);
  } // else wait for a spot to empty
  // else {
  //   printf("%s Buffer is full, waiting for an empty slot...\n", LOG_PREFIX);
  //   pthread_cond_wait(&queue_cond, &queue_mutex);
  // }
  pthread_mutex_unlock(&queue_mutex);

  // clear buffer, save the triplet, send it to commander
  memset(responses_buffer, 0, sizeof(*responses_buffer));
  sprintf(responses_buffer, "JOB <job_%d,%s> SUBMITTED", jobId, input_buffer);
  respond_to_commander(clientSocket, responses_buffer);

  printf("%s Adding job in the queue...\n", LOG_PREFIX);
  print_queue(queue);

}

void handle_set_concurrency(char* input_buffer, char *responses_buffer, int clientSocket) 
{
  remove_first_word(input_buffer);

  pthread_mutex_lock(&concurrency_mutex);

  concurrencyLevel = atoi(input_buffer);
  // if the concurrency is higher than before, we wake up the threads that are needed
  if (concurrencyLevel > numberOfRunningJobs)
  {
    for(int i = 0; i < (concurrencyLevel - numberOfRunningJobs); i++)
    {
      pthread_cond_signal(&queue_cond);
      // if we reach the limit then stop
      if (i == bufferSize) 
      {
        break;
      }
    }
  }
  pthread_mutex_unlock(&concurrency_mutex);

  memset(responses_buffer, 0, sizeof(*responses_buffer));
  sprintf(responses_buffer, "CONCURRENCY SET AT %d", concurrencyLevel);
  respond_to_commander(clientSocket, responses_buffer);
}

void handle_stop_job(char* input_buffer, char *responses_buffer, int clientSocket)
{
  remove_first_word(input_buffer);

  // save the jobId from the input
  int jobIdToStop = 0;
  sscanf(input_buffer, "job_%d", &jobIdToStop);
  if (!(jobId >= 0)) {
    respond_to_commander(clientSocket, "Invalid jobID. Must be greater than or equal to 0");
    return;
  }

  printf("%s jobIdToStop = %d\n", LOG_PREFIX, jobIdToStop);

  // see if it is in the queue
  bool found = false;
  pthread_mutex_lock(&queue_mutex);
  found = find_in_queue(queue, jobIdToStop);

  if (found)
  {
    // remove it
    remove_job_from_queue(&queue, jobIdToStop);
  }
  pthread_mutex_unlock(&queue_mutex);

  if (found)
  {
    // clear buffer and respond to the commander
    memset(responses_buffer, 0, sizeof(*responses_buffer));
    sprintf(responses_buffer, "JOB %d REMOVED", jobIdToStop);
    respond_to_commander(clientSocket, responses_buffer);
  }
  else
  { // if it's not found
    printf("%s jobIdToStop not found in queued.\n", LOG_PREFIX);

    memset(responses_buffer, 0, sizeof(*responses_buffer));
    sprintf(responses_buffer, "JOB %d NOTFOUND", jobIdToStop);
    respond_to_commander(clientSocket, responses_buffer);
  }
}

void handle_poll_jobs(char* responses_buffer, int clientSocket)
{
  memset(responses_buffer, 0, sizeof(*responses_buffer));

  pthread_mutex_lock(&queue_mutex);

  // check if there are no queued jobs
  if (queue == NULL)
  {
    strcpy(responses_buffer, "No queued jobs");
  }
  else
  {
    Queue current = queue;
    while (current != NULL)
    {
      char msg[1000];

      memset(msg, 0, sizeof(msg));
      sprintf(msg, "<job_%d,%s>\n", current->jobID, current->job);

      strcat(responses_buffer, msg);

      current = current->next;
    }
  }
  pthread_mutex_unlock(&queue_mutex);

  // send the tuples to the commander
  respond_to_commander(clientSocket, responses_buffer);
}

void notify_queued_jobs()
{
  Queue job = queue;
  while (job != NULL) {
    respond_to_commander(job->clientSocket, "JOB TERMINATED BEFORE EXECUTION");
    job = job->next;
  }
}

void* controller(void *clientSocket)
{
  int csock = *(int*)clientSocket;
  // set the buffers
  char input_buffer[COMMANDS_BUFFER];
  memset(input_buffer, 0, sizeof(input_buffer));

  char responses_buffer[COMMANDS_BUFFER];
  memset(responses_buffer, 0, sizeof(responses_buffer));

  // Read from client socket
  if (read(csock, input_buffer, sizeof(input_buffer)) < 0) {
    perror_exit("read");
    pthread_exit(NULL);
  }
 
  printf("%s: Read: *%s*\n", LOG_PREFIX, input_buffer);

  if (strcmp(input_buffer, "exit") == 0)
  {
    // wait for the jobs to finish and return their responses to the client
    // clear the queue
    // respond to the adistoixo client that SERVER TERMINATED BEFORE EXECUTION
    pthread_mutex_lock(&server_mutex);
    server_running = false;
    pthread_mutex_unlock(&server_mutex);
    
    pthread_cond_broadcast(&queue_cond);

    pthread_mutex_lock(&queue_mutex);
    // Queue job = queue;
    // while (job != NULL) {
    //   respond_to_commander(job->clientSocket, "JOB TERMINATED BEFORE EXECUTION");
    //   job = job->next;
    // }
    notify_queued_jobs();
    pthread_mutex_unlock(&queue_mutex);

    respond_to_commander(csock, "SERVER TERMINATED");

    close(csock);

    pthread_exit(NULL);

    return;
  }
  else if (strlen(input_buffer) >= 8 && strncmp(input_buffer, "issueJob", 8) == 0)
  {
    handle_issue_job(input_buffer, responses_buffer, csock);
  }
  else if (strlen(input_buffer) >= 14 && strncmp(input_buffer, "setConcurrency", 14) == 0)
  {
    handle_set_concurrency(input_buffer, responses_buffer, csock);
  }
  else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "stop", 4) == 0)
  {
    handle_stop_job(input_buffer, responses_buffer, csock);
  }
  else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "poll", 4) == 0)
  {
    handle_poll_jobs(responses_buffer, csock);
  }
  // clear input buffer
  memset(input_buffer, 0, sizeof(input_buffer));
  
  pthread_exit(NULL);
}


int main(int argc, char *argv[])
{

  if (argc != 4) {
    fprintf(stderr, "%s", correct_syntax());
    return 1;
  }

  struct sockaddr_in client;
  socklen_t clientlen;

  struct sockaddr *clientptr=(struct sockaddr *)&client;
  // struct hostent *rem;

  int sock = initialize_server(argv);
  int clientSocket;

  bufferSize = atoi(argv[2]);
  int threadPoolSize = atoi(argv[3]);

  //creates threadPoolSize worker threads
  int thread_attr, i;
  pthread_t worker_thread[threadPoolSize];
  for(i = 0; i < threadPoolSize; i++) 
  {
    thread_attr = pthread_create(&worker_thread[i], NULL, worker, NULL); //(void *)&t_args[j]
    
    if (thread_attr)
    {
      printf("ERROR; return code from pthread_create() is %d\n", thread_attr);
      perror_exit("create worker");
    }
  }

  while (1)
  {
    pthread_mutex_lock(&server_mutex);
    if (!server_running) {
      pthread_mutex_unlock(&server_mutex);
      break;
    }
    pthread_mutex_unlock(&server_mutex);

    clientlen = sizeof(client);
    if ((clientSocket = accept(sock, clientptr, &clientlen)) < 0) {
      perror_exit("accept");
    }

    pthread_t controller_thread;

    // creates a controller thread when a client is connected
    if(pthread_create(&controller_thread, NULL, controller, (void *)&clientSocket) < 0) { 
      perror("Could not create controller thread");
      continue;
    }

    pthread_join(controller_thread, NULL);

    pthread_detach(controller_thread);

  }

  // wait for the workers that are running to finish
  for (i = 0; i < threadPoolSize; i++)
  {
    pthread_join(worker_thread[i], NULL);
  }

  printf("%s: End looping\n", LOG_PREFIX);

  // clearance

  free(queue);

  close(sock);

  pthread_mutex_destroy(&queue_mutex);
  pthread_mutex_destroy(&server_mutex);
  pthread_cond_destroy(&queue_cond);

  printf("%s: Bye!\n", LOG_PREFIX);

  pthread_exit(NULL);

  return 0;
}