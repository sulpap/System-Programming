#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "correct_syntax.h"
#include "validate_command.h"
#include "job_executor_server.h"
#include "../common.h"
#include "defines.h"

#define h_addr h_addr_list[0]

void perror_exit(char *message)
{
  // printf("%s", LOG_PREFIX);
  perror(message);
  exit(EXIT_FAILURE);
}

int create_socket() {
  int sock;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror_exit("socket");
  }
  return sock;
}

struct hostent* get_server_address(const char *hostname) {
  struct hostent *rem;
  if ((rem = gethostbyname(hostname)) == NULL) 
  {
    printf("%s", LOG_PREFIX);
    herror("gethostbyname");
    exit(1);
  }
  return rem;
}

void connect_to_server(int sock, struct sockaddr_in server) {
  struct sockaddr *serverptr = (struct sockaddr*)&server;

  if (connect(sock, serverptr, sizeof(server)) < 0) 
  {
    printf("%s", LOG_PREFIX);
    perror_exit("connect");
  }
}

void save_arg(char *message, int argc, char *argv[]) {
  memset(message, 0, sizeof(char) * COMMANDS_BUFFER);
  for (int i = 3; i < argc; i++) {
    if (i > 3) { // add space before all arguments except the first 3
      strcat(message, " ");
    }
    strcat(message, argv[i]);
  }
}

// void read_response_from_executor(int sock, const char *message) 
// {
//   if (read(sock, (void *)message, COMMANDS_BUFFER) < 0) {
//     perror_exit("read");
//   }
  
//   return;
// }

void read_response_from_executor(int sock, char *message) {
  ssize_t bytes_read = read(sock, (void *)message, COMMANDS_BUFFER);
  if (bytes_read < 0) {
      perror("read");
      exit(EXIT_FAILURE);
  }
  message[bytes_read] = '\0';  // null terminator
}

int main(int argc, char *argv[]) {
  if (argc <= 3) {
    fprintf(stderr, "%s", correct_syntax());
    return 1;
  }

  int port = atoi(argv[2]);
  const char *command = argv[3];

  int sock = create_socket();
  struct hostent *rem = get_server_address(argv[1]);

  struct sockaddr_in server;
  
  server.sin_family = AF_INET;
  memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
  server.sin_port = htons(port);

  connect_to_server(sock, server);

  printf("%s Connecting to %s port %d\n", LOG_PREFIX, argv[1], port);
  printf("%s: Command = %s\n", LOG_PREFIX, command);

  // checks compatibility
  if (!validate_command(command)) {
    fprintf(stderr, "%s", correct_syntax());
    return 1;
  }

  // save the argument passed from input into the message array
  char message[COMMANDS_BUFFER];
  save_arg(message, argc, argv);

  // write it in the socket for executor to see
  if (write(sock, message, strlen(message)) < 0) {
   perror_exit("write");
  }

  // read the response from the executor and print it
  if (strcmp(message, "exit") == 0 ||
      (strlen(message) >= 8 && strncmp(message, "issueJob", 8) == 0) ||
      (strlen(message) >= 14 && strncmp(message, "setConcurrency", 14) == 0) ||
      (strlen(message) >= 4 && strncmp(message, "stop", 4) == 0) ||
      (strlen(message) >= 4 && strncmp(message, "poll", 4) == 0)) {
    // receive response from executor
    read_response_from_executor(sock, message);
    printf("%s: %s\n", LOG_PREFIX, message);
  }

  // close the socket
  close(sock);

  return 0;
}
