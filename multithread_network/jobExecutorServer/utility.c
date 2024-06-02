#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include "utility.h"
#include "defines.h"

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
  return "jobExecutorServer/bin/jobExecutorServer [portNum] [bufferSize] [threadPoolSize]\n";
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