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
#include "job_executor_server_file.h"
#include "respond_to_commander.h"
#include "../common.h"
#include "correct_syntax.h"
#include "defines.h"
#include "queue.h"
#include "remove_first_word.h"
#include "execute_command.h"

Queue queue = NULL;

// typedef struct
// {
//   int jobId;
//   char command[COMMANDS_BUFFER];
//   int clientSocket;
// } Threads_args;

typedef struct {
  int clientSocket;
  char input_buffer[COMMANDS_BUFFER];
  char responses_buffer[COMMANDS_BUFFER];
} Controller_args;

int concurrencyLevel = 1; // default
int numberOfRunningJobs = 0;
int jobId = 0;

int sock=-5; 
int clientSocket=-5;

void create_child_process(Queue *queue);

void perror_exit(char *message)
{
  printf("%s", LOG_PREFIX);
  perror(message);
  exit(EXIT_FAILURE);
}

/* it would be very bad if someone passed us an dirname like
 * "; rm *"  and we naively created a command  "ls ; rm *".
 * So..we remove everything but slashes and alphanumerics.
 */
void sanitize(char *str)
{
	char *src, *dest;
	for ( src = dest = str ; *src ; src++ )
		if ( *src == '/' || isalnum(*src) )
			*dest++ = *src;
	*dest = '\0';
}

void* print_hello_world(void* arg) {
  printf("Hello, World!\n");
  pthread_exit(NULL);
}

void initialize_server(int argc, char *argv[]) {
  int port;
  struct sockaddr_in server;
  struct sockaddr *serverptr=(struct sockaddr *)&server;

  port = atoi(argv[1]);

  // create socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror_exit("socket");
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);

  // bind socket to address
  if (bind(sock, serverptr, sizeof(server)) < 0) {
    perror_exit("bind");
  }

  // listen for connections
  if (listen(sock, 5) < 0) {
    perror_exit("listen");
  }
  printf("%s Listening for connections to port %d\n", LOG_PREFIX, port);

}

void handle_child_finished_signal(int signum)
{
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
    printf("%s: Child process with pid %d finished.\n", LOG_PREFIX, pid);

    create_child_process(&queue);
  }
  return;
}

void install_child_finish_handler()
{
  signal(SIGCHLD, handle_child_finished_signal);
}

void create_child_process(Queue *queue)
{
  Queue job = dequeue(queue); // removing job from queue to be executed

  if (job == NULL)
  {
    printf("%s No job in the queue to run.\n", LOG_PREFIX);
    return;
  }

  printf("%s Job got out of the queue ready to run %s\n", LOG_PREFIX, job->job);

  // fork one child process to execute the command
  pid_t pid = fork();
  if (pid == 0)
  {
    // child process executes the command
    execute_command(job->job);
    close(clientSocket);
  }
  else if (pid > 0)
  {
    // parent process
    numberOfRunningJobs++;
    close(clientSocket); // parent closes socket to client 
  }
  else
  {
    perror("fork failed");
  }
}

void handle_issue_job(char *input_buffer, char *responses_buffer) 
{
  jobId++;

  // remove the command (issueJob, in this case)
  remove_first_word(input_buffer);

  // put the job in the queue and save its position in it
  enqueue(&queue, input_buffer, jobId);

  // parent process
  // clear buffer, save the triplet, send it to commander
  memset(responses_buffer, 0, sizeof(responses_buffer));
  sprintf(responses_buffer, "<job_%d,%s>", jobId, input_buffer);
  respond_to_commander(clientSocket, responses_buffer);

  printf("%s Adding job in the queue...\n", LOG_PREFIX);
  print_queue(queue);

  // if we can, we execute it
  if (numberOfRunningJobs < concurrencyLevel)
  {
    create_child_process(&queue);
  }
}

void handle_set_concurrency(char* input_buffer, char *responses_buffer) 
{
  remove_first_word(input_buffer);

  concurrencyLevel = atoi(input_buffer);

  memset(responses_buffer, 0, sizeof(responses_buffer));
  sprintf(responses_buffer, "CONCURRENCY SET AT %d", concurrencyLevel);
  respond_to_commander(clientSocket, responses_buffer);
}

void handle_stop_job(char* input_buffer, char *responses_buffer)
{
  remove_first_word(input_buffer);

  // save the jobId from the input
  int jobIdToStop = atoi(input_buffer);
  if (!(jobId >= 0)) {
    respond_to_commander(clientSocket, "Invalid jobID. Must be greater than or equal to 0");
    return;
  }

  printf("%s jobIdToStop = %d\n", LOG_PREFIX, jobIdToStop);

  // see if it is in the queue
  bool found = false;
  found = find_in_queue(queue, jobIdToStop);

  if (found)
  {
    // remove it
    remove_job_from_queue(&queue, jobIdToStop);

    // clear buffer and respond to the commander
    memset(responses_buffer, 0, sizeof(responses_buffer));
    sprintf(responses_buffer, "JOB %d REMOVED", jobIdToStop);
    respond_to_commander(clientSocket, responses_buffer);
  }
  else
  { // if it's not found
    printf("%s jobIdToStop not found in queued.\n", LOG_PREFIX);

    memset(responses_buffer, 0, sizeof(responses_buffer));
    sprintf(responses_buffer, "JOB %d NOTFOUND", jobIdToStop);
    respond_to_commander(clientSocket, responses_buffer);
  }
}

void handle_poll_jobs(char* responses_buffer)
{
  memset(responses_buffer, 0, sizeof(responses_buffer));

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
  // send the tuples to the commander
  respond_to_commander(clientSocket, responses_buffer);

}

// void* controller(void* args)
// {
//   printf("this is controller thread\n");
 
//   Controller_args* controller_args = (Controller_args*) args;
//   int clientSocket = controller_args->clientSocket;
//   // char* input_buffer = controller_args->input_buffer;
//   // char* responses_buffer = controller_args->responses_buffer;

//   //printf("clientSocket: %d, input_buffer: %s, responses_buffer: %s\n", clientSocket, controller_args->input_buffer, controller_args->responses_buffer);

//   free(controller_args);

//   // we have something to read from the socket
//   if (read(clientSocket, controller_args->input_buffer, sizeof(controller_args->input_buffer)) < 0) {
//     perror_exit("read");
//   }
//   printf("clientSocket: %d, input_buffer: %s, responses_buffer: %s\n", clientSocket, controller_args->input_buffer, controller_args->responses_buffer);

//   // // sanitize the input buffer to remove any harmful characters
//   // sanitize(input_buffer);

//   // if (strcmp(input_buffer, "") == 0)
//   // {
//   //   sleep(1); // fix this
//   // }
//   // else
//   // {

//   //   printf("%s: Read: *%s*\n", LOG_PREFIX, input_buffer);

//   //   if (strcmp(input_buffer, "exit") == 0)
//   //   {
//   //     // wait for the jobs to finish and return their responses to the client
//   //     // clear the queue
//   //     // respond to the adistoixo client that SERVER TERMINATED BEFORE EXECUTION
//   //     respond_to_commander(clientSocket, "SERVER TERMINATED");

//   //     close(clientSocket);
//   //     free(queue);
//   //     close(sock);
//   //     delete_job_executor_server_file(); 
//   //     free(controller_args);

//   //     return;
//   //   }
//   //   else if (strlen(input_buffer) >= 8 && strncmp(input_buffer, "issueJob", 8) == 0)
//   //   {
//   //     handle_issue_job(input_buffer,responses_buffer);

//   //   }
//   //   else if (strlen(input_buffer) >= 14 && strncmp(input_buffer, "setConcurrency", 14) == 0)
//   //   {
//   //     handle_set_concurrency(input_buffer, responses_buffer);

//   //   }
//   //   else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "stop", 4) == 0)
//   //   {
//   //     handle_stop_job(input_buffer, responses_buffer);
      
//   //   }
//   //   else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "poll", 4) == 0)
//   //   {
//   //     handle_poll_jobs(responses_buffer);
      
//   //   }
//   //   // clear input buffer
//   //   memset(input_buffer, 0, sizeof(input_buffer));

//   //   // close the connection socket
//   //   close(clientSocket);
//   // }
// }

void* controller(void* args)
{
  printf("this is controller thread\n");

  Controller_args* controller_args = (Controller_args*) args;
  int client_socket = controller_args->clientSocket;
  char* input_buffer = controller_args->input_buffer;
  char* responses_buffer = controller_args->responses_buffer;

  // Read from client socket
  if (read(client_socket, input_buffer, sizeof(controller_args->input_buffer)) < 0) {
    perror_exit("read");
    pthread_exit(NULL);
  }

  // Process input
  if (strcmp(input_buffer, "") == 0)
  {
    sleep(1); // fix this
  }
  else
  {
    printf("%s: Read: *%s*\n", LOG_PREFIX, input_buffer);

    if (strcmp(input_buffer, "exit") == 0)
    {
      // wait for the jobs to finish and return their responses to the client
      // clear the queue
      // respond to the adistoixo client that SERVER TERMINATED BEFORE EXECUTION
      respond_to_commander(client_socket, "SERVER TERMINATED");
      free(controller_args);
      printf("after free\n");
      close(client_socket);
      printf("after close\n");


      return;
    }
    else if (strlen(input_buffer) >= 8 && strncmp(input_buffer, "issueJob", 8) == 0)
    {
      handle_issue_job(input_buffer, responses_buffer);
    }
    else if (strlen(input_buffer) >= 14 && strncmp(input_buffer, "setConcurrency", 14) == 0)
    {
      handle_set_concurrency(input_buffer, responses_buffer);
    }
    else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "stop", 4) == 0)
    {
      handle_stop_job(input_buffer, responses_buffer);
    }
    else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "poll", 4) == 0)
    {
      handle_poll_jobs(responses_buffer);
    }
    // clear input buffer
    memset(input_buffer, 0, sizeof(controller_args->input_buffer));
  }

  // close the connection socket
  //close(client_socket);
  //pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
  create_job_executor_server_file(); //////////////////////

  if (argc != 4) {
    fprintf(stderr, "%s", correct_syntax());
    return 1;
  }

  struct sockaddr_in client;
  socklen_t clientlen;

  struct sockaddr *clientptr=(struct sockaddr *)&client;
  struct hostent *rem;

  initialize_server(argc, argv);

  int bufferSize = atoi(argv[2]);
  int threadPoolSize = atoi(argv[3]);

  //creates threadPoolSize worker threads
  int thread_attr, i;
  pthread_t worker_thread[threadPoolSize];
  for(i = 0; i < threadPoolSize; i++) 
  {
    thread_attr = pthread_create(&worker_thread[i], NULL, print_hello_world, NULL); //(void *)&t_args[j]
    
    if (thread_attr)
    {
      printf("ERROR; return code from pthread_create() is %d\n", thread_attr);
      perror_exit("create worker");
    }
  }

  for (i = 0; i < threadPoolSize; i++)
  {
    pthread_join(worker_thread[i], NULL);
  }

  // set the buffers
  char input_buffer[COMMANDS_BUFFER];
  memset(input_buffer, 0, sizeof(input_buffer));

  char responses_buffer[COMMANDS_BUFFER];
  memset(responses_buffer, 0, sizeof(responses_buffer));

  // handler for terminated jobs
  install_child_finish_handler();

  while (1)
  {
    clientlen = sizeof(client);
    if ((clientSocket = accept(sock, clientptr, &clientlen)) < 0) {
      perror_exit("accept");
    }

    pthread_t controller_thread;

    // Allocate and initialize the controller args
    Controller_args* controller_args = (Controller_args*) malloc(sizeof(Controller_args));
    controller_args->clientSocket = clientSocket;
    memset(controller_args->input_buffer, 0, sizeof(controller_args->input_buffer));
    memset(controller_args->responses_buffer, 0, sizeof(controller_args->responses_buffer));

    // creates a controller thread when a client is connected
    if(pthread_create(&controller_thread, NULL, controller, controller_args) < 0) { 
      perror("Could not create controller thread");
      free(controller_args);
      continue;
    }

    pthread_join(controller_thread, NULL);
    printf("after join\n");

    pthread_detach(controller_thread);
    printf("after detatch\n");

    
    // // we have something to read from the socket
    // if (read(clientSocket, input_buffer, sizeof(input_buffer)) < 0) {
    //   perror_exit("read");
    // }

    // if (strcmp(input_buffer, "") == 0)
    // {
    //   sleep(1); // fix this
    // }
    // else
    // {

    //   printf("%s: Read: *%s*\n", LOG_PREFIX, input_buffer);

    //   if (strcmp(input_buffer, "exit") == 0)
    //   {
    //     // wait for the jobs to finish and return their responses to the client
    //     // clear the queue
    //     // respond to the adistoixo client that SERVER TERMINATED BEFORE EXECUTION
    //     respond_to_commander(clientSocket, "SERVER TERMINATED");
    //     close(clientSocket);
    //     break;
    //   }
    //   else if (strlen(input_buffer) >= 8 && strncmp(input_buffer, "issueJob", 8) == 0)
    //   {
    //     handle_issue_job(input_buffer,responses_buffer);

    //   }
    //   else if (strlen(input_buffer) >= 14 && strncmp(input_buffer, "setConcurrency", 14) == 0)
    //   {
    //     handle_set_concurrency(input_buffer, responses_buffer);

    //   }
    //   else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "stop", 4) == 0)
    //   {
    //     handle_stop_job(input_buffer, responses_buffer);
        
    //   }
    //   else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "poll", 4) == 0)
    //   {
    //     handle_poll_jobs(responses_buffer);
        
    //   }
    //   // clear input buffer
    //   memset(input_buffer, 0, sizeof(input_buffer));

    //   // close the connection socket
    //   close(clientSocket);
    // }

  }

  printf("%s: end looping\n", LOG_PREFIX);

 //detach workers?

  // clearance

  free(queue);

  close(sock);

  delete_job_executor_server_file(); // needs??????????????

  printf("%s: Bye!\n", LOG_PREFIX);

  pthread_exit(NULL);

  return 0;
}
