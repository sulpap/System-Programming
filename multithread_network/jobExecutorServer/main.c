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
#include "job_executor_server_file.h"
#include "respond_to_commander.h"
#include "../common.h"
#include "defines.h"
#include "queue.h"
#include "remove_first_word.h"
#include "execute_command.h"
#include "../pipes.h"

Queue queue = NULL;

int Concurrency = 1; // default
int numberOfRunningJobs = 0;
int jobId = 0;

int sock=-5; 
int newsock=-5;

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
// void sanitize(char *str)
// {
// 	char *src, *dest;
// 	for ( src = dest = str ; *src ; src++ )
// 		if ( *src == '/' || isalnum(*src) )
// 			*dest++ = *src;
// 	*dest = '\0';
// }

void initialize_server(int argc, char *argv[]) {
  int port;
  struct sockaddr_in server;
  struct sockaddr *serverptr=(struct sockaddr *)&server;

  // if (argc != 2) {
  //   printf("%s Please give port number\n", LOG_PREFIX);          // needed??????
  //   exit(1);
  // }
  port = atoi(argv[1]);

  // bufferSize = atoi(argv[2]);
  // set_buffer_size(bufferSize);
  // printf("buffer size: %d", COMMANDS_BUFFER); // debug

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
    close(newsock);
  }
  else if (pid > 0)
  {
    // parent process
    numberOfRunningJobs++;
    close(newsock); // parent closes socket to client 
  }
  else
  {
    perror("fork failed");
  }
}

int main(int argc, char *argv[])
{
  create_job_executor_server_file();

  // int port; // sock, newsock, bufferSize;
  // struct sockaddr_in server, 
  struct sockaddr_in client;
  socklen_t clientlen;

  // struct sockaddr *serverptr=(struct sockaddr *)&server;
  struct sockaddr *clientptr=(struct sockaddr *)&client;
  struct hostent *rem;

  initialize_server(argc, argv);

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
    if ((newsock = accept(sock, clientptr, &clientlen)) < 0) {
      perror_exit("accept");
    }
    // we have something to read from the socket
    if (read(newsock, input_buffer, sizeof(input_buffer)) < 0) {
      perror_exit("read");
    }

    if (strcmp(input_buffer, "") == 0)
    {
      sleep(1); // fix this
    }
    else
    {

      printf("%s: Read: *%s*\n", LOG_PREFIX, input_buffer);

      if (strcmp(input_buffer, "exit") == 0)
      {
        respond_to_commander(sock, "SERVER TERMINATED");
        close(newsock);
        break;
      }
      else if (strlen(input_buffer) >= 8 && strncmp(input_buffer, "issueJob", 8) == 0) // giati exei keno??????????????????
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
        respond_to_commander(newsock, responses_buffer);

        printf("%s Adding job in the queue...\n", LOG_PREFIX);
        print_queue(queue);

        // if we can, we execute it
        if (numberOfRunningJobs < Concurrency)
        {
          create_child_process(&queue);
        }
      }
      else if (strlen(input_buffer) >= 14 && strncmp(input_buffer, "setConcurrency", 14) == 0)
      {

        remove_first_word(input_buffer);

        Concurrency = atoi(input_buffer);

        sprintf(responses_buffer, "CONCURRENCY SET AT %d", Concurrency);

        respond_to_commander(newsock, responses_buffer);

      }
      else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "stop", 4) == 0)
      {

        remove_first_word(input_buffer);

        // save the jobId from the input
        int jobIdToStop = 0;
        sscanf(input_buffer, "job_%d", &jobIdToStop);

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
          respond_to_commander(sock, responses_buffer);
        }
        else
        { // if it's not found
          printf("%s jobIdToStop not found in queued.\n", LOG_PREFIX);

          memset(responses_buffer, 0, sizeof(responses_buffer));
          sprintf(responses_buffer, "JOB %d NOTFOUND", jobIdToStop);
          respond_to_commander(sock, responses_buffer);
        
        }
      }
      else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "poll", 4) == 0)
      {

        remove_first_word(input_buffer);

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
            sprintf(msg, "job_%d,%s\n", current->jobID, current->job);

            strcat(responses_buffer, msg);

            current = current->next;
          }
        }
        // send response (the buffer with all the triplets) to the commander
        respond_to_commander(sock, responses_buffer);
        
      }
      // clear input buffer
      memset(input_buffer, 0, sizeof(input_buffer));

      // close the connection socket
      close(newsock);
    }
  }

  printf("%s: end looping\n", LOG_PREFIX);

  // clearance

  free(queue);

  close(sock);

  delete_job_executor_server_file(); // needs??????????????

  printf("%s: Bye!\n", LOG_PREFIX);

  return 0;
}