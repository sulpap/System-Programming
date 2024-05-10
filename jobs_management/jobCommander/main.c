#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "correct_syntax.h"
#include "validate_command.h"
#include "job_executor_server.h"
#include "../common.h"
#include "defines.h"
#include "../pipes.h"

void create_output_pipe() {
  create_pipe(COMMANDS_PIPE);
}

void create_input_pipe() {
  create_pipe(RESPONSES_PIPE);
}

int open_output_pipe() {
  return open_pipe(COMMANDS_PIPE, O_WRONLY);
}

int open_input_pipe() {
  return open_pipe(RESPONSES_PIPE, O_RDONLY);
}

void read_response_from_executor(int fd_input, const char *message) {
  read(fd_input, (void *)message, COMMANDS_BUFFER);

  return;
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "%s", correct_syntax());
    return 1;
  }

  const char *command = argv[1];

  // prints received command
  printf("%s: Command = %s\n", LOG_PREFIX, command);

  // checks compatibility
  if (!validate_command(command)) {
    fprintf(stderr, "%s", correct_syntax());
    return 1;
  }

  // if the server doesn't exist, creates it
  if (!job_executor_server_running()) {
    start_job_executor_server();
  }

  // create the pipes
  create_output_pipe();
  create_input_pipe();

  // open them
  int fd_output = -1;

  fd_output = open_output_pipe();

  int fd_input = -1;

  fd_input = open_input_pipe();

  // save the argument passed from input into the message array
  char message[COMMANDS_BUFFER];
  memset(message, 0, sizeof(message));
  for (int i = 1; i < argc; i++) {
    if (i >= 2) {
      strcat(message, " ");
    }
    strcat(message, argv[i]);
  }

  // write it in the pipe for executor to see
  write(fd_output, message, strlen(message) + 1);

  close(fd_output);

  // read the response from the executor and print it
  if (strcmp(message, "exit") == 0 ||
      (strlen(message) >= 8 && strncmp(message, "issueJob", 8) == 0) ||
      (strlen(message) >= 4 && strncmp(message, "stop", 4) == 0) ||
      (strlen(message) >= 4 && strncmp(message, "poll", 4) == 0)) {
    read_response_from_executor(fd_input, message);
    printf("%s: %s\n", LOG_PREFIX, message);
  }

  close(fd_input);

  return 0;
}
