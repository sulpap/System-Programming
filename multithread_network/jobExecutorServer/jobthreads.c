// typedef struct {    // thread struct
//   int jobId;
//   char job[COMMANDS_BUFFER];
//   int clientSocket;
// } Job;

// void *worker_thread_function(void *arg) {
//   while (1) {
//     pthread_mutex_lock(&queue_mutex);

//     while (queue == NULL) {
//         pthread_cond_wait(&queue_not_empty, &queue_mutex);
//     }

//     Queue job = dequeue(&queue);
//     pthread_cond_signal(&queue_not_full);

//     pthread_mutex_unlock(&queue_mutex);

//     if (job != NULL) {
//       // Execute the job
//       pid_t pid = fork();
//       if (pid == 0) {
//         // Child process
//         char output_file[256];
//         sprintf(output_file, "%d.output", getpid());
//         int fd = open(output_file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
//         dup2(fd, STDOUT_FILENO);
//         close(fd);

//         execlp("/bin/sh", "sh", "-c", job->job, (char *)NULL);
//         perror("execlp");
//         exit(1);
//       } else if (pid > 0) {
//         // Parent process
//         int status;
//         waitpid(pid, &status, 0);

//         // Read output and send to client
//         char output_file[256];
//         sprintf(output_file, "%d.output", pid);
//         FILE *file = fopen(output_file, "r");
//         if (file != NULL) {
//           char buffer[1024];
//           memset(buffer, 0, sizeof(buffer));
//           fread(buffer, 1, sizeof(buffer), file);
//           fclose(file);

//           char response[COMMANDS_BUFFER];
//           sprintf(response, "-----job_%d output start------\n%s\n-----job_%d output end------\n", job->jobID, buffer, job->jobID);
//           send(job->clientSocket, response, strlen(response), 0);
//           close(job->clientSocket);
//           remove(output_file);
//           }

//         free(job);
//       } else {
//         perror("fork failed");
//       }
//     }
//   }
//   return NULL;
// }

// void *controller_thread_function(void *arg) {
//   int newsock = *(int *)arg;
//   free(arg);

//   char input_buffer[COMMANDS_BUFFER];
//   memset(input_buffer, 0, sizeof(input_buffer));

//   if (read(newsock, input_buffer, sizeof(input_buffer)) < 0) {
//     perror_exit("read");
//   }

//   if (strcmp(input_buffer, "exit") == 0) {
//     respond_to_commander(newsock, "SERVER TERMINATED");
//     close(newsock);
//   } else if (strncmp(input_buffer, "issueJob", 8) == 0) {
//     jobId++;

//     remove_first_word(input_buffer);

//     Job *newJob = malloc(sizeof(Job));
//     newJob->jobId = jobId;
//     strcpy(newJob->job, input_buffer);
//     newJob->clientSocket = newsock;

//     pthread_mutex_lock(&queue_mutex);

//     while (queue_size(queue) >= bufferSize) {
//       pthread_cond_wait(&queue_not_full, &queue_mutex);
//     }

//     enqueue(&queue, newJob);
//     pthread_cond_signal(&queue_not_empty);

//     pthread_mutex_unlock(&queue_mutex);

//     char response[COMMANDS_BUFFER];
//     sprintf(response, "JOB %d,%s SUBMITTED", jobId, input_buffer);
//     respond_to_commander(newsock, response);
//   }

//   close(newsock);
//   return NULL;
// }
