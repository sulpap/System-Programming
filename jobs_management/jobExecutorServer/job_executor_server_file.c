#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "job_executor_server_file.h"

static const char *job_executor_server_file_name = "jobExecutorServer.txt";

void create_job_executor_server_file() {

  FILE *file;

  file = fopen(job_executor_server_file_name, "w");

  if (file == NULL) {
    fprintf(stderr,  "Error opening file %s!\n", job_executor_server_file_name);
    exit(1);
  }

  pid_t pid = getpid();
  fprintf(file, "%d", pid);

  fclose(file);

  return;
}

void delete_job_executor_server_file() {
  remove(job_executor_server_file_name);
}
