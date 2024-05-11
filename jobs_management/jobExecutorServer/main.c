#include <stdio.h>
#include <unistd.h>
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
#include "queue.h"

int numberOfRunningJobs = 0;
typedef struct runningNode {
  pid_t pid;
  int jobId;
  int queuePosition;
  char command[COMMANDS_BUFFER];
} RunningNode;

RunningNode *running = NULL;

Queue queue = NULL;
Queue running_queue = NULL; // αλλη δομη queue που αποθηκευει και pid??
int Concurrency = 5; // default

int jobId = 0;

bool remove_pid(pid_t pid);
void create_child_process(Queue *queue);

void create_input_pipe() {
  create_pipe(COMMANDS_PIPE);
}

void create_output_pipe() {
  create_pipe(RESPONSES_PIPE);
}

int open_input_pipe() {
  return open_pipe(COMMANDS_PIPE, O_RDONLY);
}

int open_output_pipe() {
  return open_pipe(RESPONSES_PIPE, O_WRONLY);
}

void handle_child_finished_signal(int signum) {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    if ( remove_pid(pid) == false ){
      break;
    }

    printf("%s: child process with pid %d finished\n", LOG_PREFIX, pid);

    create_child_process(&queue);
  }
  return;
}

void install_child_finish_handler() {
  signal(SIGCHLD, handle_child_finished_signal);
}

bool remove_pid(pid_t pid) {
    int found = 0;
    for(int i = 0; i < numberOfRunningJobs; i++) {
        if (running[i].pid == pid) {
            // Shift elements to the left to overwrite the deleted element
            memmove(&running[i], &running[i + 1], (numberOfRunningJobs - i - 1) * sizeof(RunningNode));
            // decrease the size of the array by 1
            numberOfRunningJobs--;
            running = (RunningNode *)realloc(running, numberOfRunningJobs * sizeof(RunningNode));  // Reallocate memory for one less element
            if (running == NULL && numberOfRunningJobs > 0) {
                printf("%s Memory allocation failed!\n", LOG_PREFIX);
                exit(1);
            }
            printf("%s Job pid = %d removed from running jobs successfully.\n", LOG_PREFIX, pid);
            found = 1;
            break;
        }
    }
    if (!found) {
      printf("%s Job %d not found in the running jobs array.\n", LOG_PREFIX, pid);
      //exit(1);
      return false;
    }

    return true;
}

void create_child_process(Queue *queue) {
  int queuePosition = counter(queue);
  Queue job = dequeue(queue); //removing job from queue to be executed

  printf("%s Printing the queue...\n", LOG_PREFIX);
  print_queue(*queue);

  if (job == NULL) {
    printf("%s no job in the queue to run\n", LOG_PREFIX);
    return;
  }

  printf("%s JOB got out of the queue ready to run %s\n", LOG_PREFIX, job->job);

  // fork one child process to execute the command
  pid_t pid = fork();
  if (pid == 0) {
      // child process

      execute_command(job->job);
  } else {
      numberOfRunningJobs++;
      running = (RunningNode *)realloc(running, numberOfRunningJobs * sizeof(RunningNode));
      if (running == NULL) {
        printf("%s Memory allocation failed!\n", LOG_PREFIX);
        exit(1);
      }

      running[numberOfRunningJobs - 1].pid = pid;
      running[numberOfRunningJobs - 1].jobId = jobId;
      strcpy(running[numberOfRunningJobs - 1].command, job->job);
      running[numberOfRunningJobs - 1].queuePosition = queuePosition;


      printf("%s RUNNING pid array: ", LOG_PREFIX);
      for(int t = 0; t < numberOfRunningJobs; t++){
        printf("pid = %d jobId = %d ", running[t].pid, running[t].jobId);
      }
      printf("\n");
  }
}

int main(int argc, char *argv[]) {
  create_job_executor_server_file();

  create_input_pipe();
  create_output_pipe();

  char input_buffer[COMMANDS_BUFFER];
  memset(input_buffer, 0, sizeof(input_buffer));

  char responses_buffer[COMMANDS_BUFFER];
  memset(responses_buffer, 0, sizeof(responses_buffer));

  int fd_input = -1;

  printf("%s: About to open input pipe\n", LOG_PREFIX);
  fd_input = open_input_pipe();

  int fd_output = -1;
  fd_output = open_output_pipe();

  int queuePosition = 0;

  install_child_finish_handler();

  while(1) {
    // we have something to read from the pipe
    read(fd_input, input_buffer, sizeof(input_buffer));
    if (strcmp(input_buffer, "") == 0) {
      sleep(1); // sleep for 1 second
    } else {
      printf("%s: Read: *%s*\n", LOG_PREFIX, input_buffer);

      if (strcmp(input_buffer, "exit") == 0) {
        respond_to_commander(fd_output, "jobExecutorServer terminated");
        break;
      } else if (strlen(input_buffer) >= 8 && strncmp(input_buffer, "issueJob", 8) == 0) {
        jobId++;

        remove_first_word(input_buffer);

        enqueue(&queue, input_buffer, jobId); // puts at the end/back of the queue
        queuePosition = counter(queue)-1;

        // parent process
        memset(responses_buffer, 0, sizeof(responses_buffer));
        sprintf(responses_buffer, "job_%d,%s,%d", jobId, input_buffer, queuePosition);
        respond_to_commander(fd_output, responses_buffer);

        printf("%s Printing the queue...\n", LOG_PREFIX);
        print_queue(queue);

        if (numberOfRunningJobs < Concurrency) {
          create_child_process(&queue);
        }

      } else if (strlen(input_buffer) >= 14 && strncmp(input_buffer, "setConcurrency", 14) == 0) {

        remove_first_word(input_buffer);

        Concurrency = atoi(input_buffer); // doesn't need to return anything to jobCommander

      } else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "stop", 4) == 0) {

        remove_first_word(input_buffer);

        int jobIdToStop = 0;
        sscanf(input_buffer, "job_%d", &jobIdToStop);
        printf("%s jobIdToStop = %d\n", LOG_PREFIX, jobIdToStop);

        bool found = false;
        int i = 0;
        while(!found && i < numberOfRunningJobs) {
          if (running[i].jobId == jobIdToStop) {
            found = true;
          }
          i++;
        }

        if (found) {
          int pid = running[i - 1].pid;

          printf("%s sending SIGKILL to process with pid %d\n", LOG_PREFIX, pid);
          kill(pid, SIGTERM);

          memset(responses_buffer, 0, sizeof(responses_buffer));
          sprintf(responses_buffer, "job_%d terminated", jobIdToStop);
          respond_to_commander(fd_output, responses_buffer);
        } else {
          printf("%s jobIdToStop not found in running\n", LOG_PREFIX);

          // we need to check whether it is in the queue
          // It should be in the queue.
          remove_job(&queue, jobIdToStop);

          memset(responses_buffer, 0, sizeof(responses_buffer));
          sprintf(responses_buffer, "job_%d removed", jobIdToStop);
          respond_to_commander(fd_output, responses_buffer);
        }
      } else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "poll", 4) == 0) {

        remove_first_word(input_buffer);

        if (strcmp(input_buffer, "running") == 0) {

          memset(responses_buffer, 0, sizeof(responses_buffer));
          for(int i = 0; i < numberOfRunningJobs; i++) {
            char tripletMessage[1000];

            memset(tripletMessage, 0, sizeof(tripletMessage));
            sprintf(tripletMessage, "job_%d,%s,%d\n", running[i].jobId, running[i].command, running[i].queuePosition);

            strcat(responses_buffer, tripletMessage);
          }
          respond_to_commander(fd_output, responses_buffer);

        } else if (strcmp(input_buffer, "queued")) {
          // get_print_queue(queue, &responses_buffer);
          // respond_to_commander(fd_output, responses_buffer);
          memset(responses_buffer, 0, sizeof(responses_buffer));
          for (int i = 0; i < counter(queue); i++ ) {
            char tripletMessage[1000];

            memset(tripletMessage, 0, sizeof(tripletMessage));
            sprintf(tripletMessage, "job_%d,%s,%d\n", queue->jobID, queue->job, i);

            strcat(responses_buffer, tripletMessage);
          }
          respond_to_commander(fd_output, responses_buffer);

        }

      }

      memset(input_buffer, 0, sizeof(input_buffer));
    }
  }

  printf("%s: end looping\n", LOG_PREFIX);

  free(running);

  close(fd_input);
  close(fd_output);

  delete_job_executor_server_file();

  printf("%s: Executor Server: Bye!\n", LOG_PREFIX);

  return 0;
}
