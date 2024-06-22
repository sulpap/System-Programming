#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../include/utility.h"
#include "../include/defines.h"

void perror_exit(char *message)
{
  printf("%s", LOG_PREFIX);
  perror(message);
  exit(EXIT_FAILURE);
}

void sanitize(char *str)
{
	char *src, *dest;
	for ( src = dest = str ; *src ; src++ )
		if ( *src == '/' || isalnum(*src) )
			*dest++ = *src;
	*dest = '\0';
}

// gives the user instructions on running the program
const char *correct_syntax() {
  return "./bin/jobExecutorServer [portNum] [bufferSize] [threadPoolSize]\n";
}

void remove_first_word(char *sentence) {
  int i;
  int len = strlen(sentence);

  // find the position of the first space
  for (i = 0; i < len; i++) {
    if (sentence[i] == ' ') {
      break;
    }
  }

  // move the rest of the string forward
  memmove(sentence, sentence + i + 1, len - i);

  // add null terminator to the end of the modified string
  sentence[len - i] = '\0';
}

int initialize_server(char *argv[]) {
  int port, sock;
  struct sockaddr_in server;
  struct sockaddr *serverptr=(struct sockaddr *)&server;

  port = atoi(argv[1]);

  // create socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror_exit("socket");
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);

  // bind socket to address
  if (bind(sock, serverptr, sizeof(server)) < 0) {
    perror_exit("bind");
  }

  // listen for connections
  if (listen(sock, 5) < 0) {
    perror_exit("listen");
  }
  printf("%s Listening for connections to port %d\n", LOG_PREFIX, port);

  return sock;

}