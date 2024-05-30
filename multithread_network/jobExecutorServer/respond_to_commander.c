#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "respond_to_commander.h"

void respond_to_commander(int sock, const void *message) {
  size_t message_length = strlen(message);
  if (write(sock, message, message_length) != message_length) {
    perror("write");
  }

  return;
}

// write((sock, message, sizeof(message) + 1));

// send(sock, message, strlen(message), 0);