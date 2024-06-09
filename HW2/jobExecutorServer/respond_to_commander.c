#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "respond_to_commander.h"

void respond_to_commander(int sock, const void *message) {
  size_t message_length = strlen(message);
  if ((size_t)write(sock, message, message_length) != message_length) {
    perror("write");
  }

  return;
}

// send(sock, message, strlen(message), 0);