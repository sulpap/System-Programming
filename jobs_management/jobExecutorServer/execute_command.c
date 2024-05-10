#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "execute_command.h"

#define MAXIMUM_NUMBER_OF_TOKENS 1000

void execute_command(char *input_buffer) {
  const char delimiter[2] = " ";
  char *token = NULL;
  char *tokens[MAXIMUM_NUMBER_OF_TOKENS];

  token = strtok(input_buffer, delimiter);

  // Walk through other tokens
  int i = 0;
  while (token != NULL && i < MAXIMUM_NUMBER_OF_TOKENS) {
      tokens[i] = token;
      token = strtok(NULL, " ");
      i++;
  }

  tokens[i] = NULL;

  execvp(tokens[0], tokens);

  perror("execvp");
}
