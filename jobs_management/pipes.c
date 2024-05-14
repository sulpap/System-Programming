#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "pipes.h"

void create_pipe(char *pipe) {
  if (access(pipe, F_OK) != -1) {
    // pipe already exists. We don't create it.
  } else {
    // pipe does not exist. We create it.
    mkfifo(pipe, 0666);
  }
}

int open_pipe(char *pipe, int options) {
  int fd = -1;
  while(fd == -1) {
    // this is blocking waiting for input on the pipe
    fd = open(pipe, options);

    if (fd == -1) {
      fprintf(stderr, "opening pipe %s failed\n", pipe);
      sleep(1);
    } else {
      break;
    }
  }
  return fd;
}
