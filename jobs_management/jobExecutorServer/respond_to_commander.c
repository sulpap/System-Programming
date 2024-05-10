#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "respond_to_commander.h"

void respond_to_commander(int fd_output, const void *message) {
  write(fd_output, message, strlen(message) + 1);

  return;
}
