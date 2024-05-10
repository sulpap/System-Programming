
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include "job_executor_server.h"
#include "defines.h"

pid_t get_job_executor_server_pid(const char *filename) {
  FILE *file;
  file = fopen(filename, "r");
  if (file == NULL) {
    return -1;
  }

  pid_t result = -1;
  char line[100];
  char *endptr;

  if (fgets(line, sizeof(line), file) != NULL) {
    result = (pid_t)strtol(line, &endptr, 10);
  }

  fclose(file);

  return result;
}

int job_executor_server_running() {
  const char *filename = "jobExecutorServer.txt";

  if (access(filename, F_OK) != -1) {
    // pid_t pid = get_job_executor_server_pid(filename);

    // if (kill(pid, 0) == 0) {
    //   // process is running
    //   return 1;
    // } else {
    //   // process is not running. PID file is stale. I will delete it
    //   remove(filename);

    //   return 0;
    // }

    // file has been found and it has a valid PID
    return 1;
  } else {
    // not job execution server file has been found.
    return 0;
  }
}

void start_job_executor_server() {
  printf("%s: I will start the jobExecutorServer\n", LOG_PREFIX);

  pid_t pid;

  pid = fork();

  if (pid < 0) {
    // error occurred
    fprintf(stderr, "Fork failed to start jobExecutorServer\n");
    exit(1);
  }

  if (pid == 0) {
    // Child process
    // Execute a new program
    execlp("./jobExecutorServer", "jobExecutorServer", NULL);

    // If execlp returns it means that there was an error
    perror("exec");

    exit(1);
  } else {
    printf("%s: I have started the executor\n", LOG_PREFIX);
  }

  return;
}
