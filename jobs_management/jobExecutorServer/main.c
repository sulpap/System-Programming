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

int size = 0;
pid_t *running = NULL;
Queue queue = NULL;
// Queue running_queue = NULL; // !!! αυτο θελει άλλη δομη queue που αποθηκευει και pid??
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
    for(int i = 0; i < size; i++) {
        if (running[i] == pid) {
            // Shift elements to the left to overwrite the deleted element
            memmove(&running[i], &running[i + 1], (size - i - 1) * sizeof(int));
            // decrease the size of the array by 1
            size--;
            running = (int *)realloc(running, size * sizeof(int));  // Reallocate memory for one less element
            if (running == NULL && size > 0) {
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
      size++;
      running = (pid_t *)realloc(running, size * sizeof(pid_t));
      if (running == NULL) {
        printf("%s Memory allocation failed!\n", LOG_PREFIX);
        exit(1);
      }

      running[size-1] = pid;

      printf("%s RUNNING pid array: ", LOG_PREFIX);
      for(int t = 0; t < size; t++){
        printf("%d ", running[t]);
      }
      printf("\n");
      
      // int jobId = get_first_job(&queue);
      // enqueue(&running_queue, buf, jobId);

      // printf("\n\nrunning queue:\n:");
      // print_queue(running_queue);
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

  int jobId = 0;
  int queuePosition = 0;
  int Concurrency = 1; // default

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

        if (size < Concurrency) {
        //if (counter(running_queue) < Concurrency) {
          create_child_process(&queue);
        }

      } else if (strlen(input_buffer) >= 14 && strncmp(input_buffer, "setConcurrency", 14) == 0) {

        remove_first_word(input_buffer);

        Concurrency = atoi(input_buffer); // doesn't need to return anything to jobCommander

      } else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "stop", 4) == 0) {
        
        remove_first_word(input_buffer);

        int parameter = atoi(input_buffer);
        bool flag = false;

        for(int i = 0; i < size; i++){
          // if the job is running, stop it and send message to commander
          
          if (running[i] == parameter) {
            remove_pid(running[i]);
            sprintf(responses_buffer, "job_%d terminated", jobId);
            respond_to_commander(fd_output, responses_buffer);
            flag = true; // true: we found it
            break;
          }
        }

        //  if it's queued, remove it and send message to commander
        if (flag == false) {
          remove_job(queue, input_buffer);
          sprintf(responses_buffer, "job_%d removed", jobId);
          respond_to_commander(fd_output, responses_buffer);       
        }

      } else if (strlen(input_buffer) >= 4 && strncmp(input_buffer, "poll", 4) == 0) {

        remove_first_word(input_buffer);

        if (strcmp(input_buffer, "running")) {

          printf("\nI run\n\n");

        } else if (strcmp(input_buffer, "queued")) {
          get_print_queue(queue, &responses_buffer);
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
