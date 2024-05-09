#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern char *PIPE1;
extern char *PIPE2;
extern int SIZE;
extern char *jobExecutorServer_file;
extern int Concurrency;

void jobExecutorServer();
void jobCommander(char**, int);
void respond_string(char *);
void respond_array(char **);