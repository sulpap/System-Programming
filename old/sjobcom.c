//job commander
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

void issueJob(int sockfd, char *job) {
    char buffer[1024];
    sprintf(buffer, "issueJob %s", job);
    send(sockfd, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
}

void setConcurrency(int sockfd, int N) {
    char buffer[1024];
    sprintf(buffer, "setConcurrency %d", N);
    send(sockfd, buffer, strlen(buffer), 0);
}

void stop(int sockfd, char *jobID) {
    char buffer[1024];
    sprintf(buffer, "stop %s", jobID);
    send(sockfd, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
}

void poll(int sockfd, char *status) {
    char buffer[1024];
    sprintf(buffer, "poll %s", status);
    send(sockfd, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
}

void exitCommand(int sockfd) {
    char buffer[1024];
    send(sockfd, "exit", strlen("exit"), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    // Δημιουργία socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Εκχώρηση IP, Θύρας
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Σύνδεση με τον server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connection with the server failed");
        exit(EXIT_FAILURE);
    }

    // Παραδείγματα εντολών
    issueJob(sockfd, "ls -l /path/to/directory1");
    issueJob(sockfd, "wget aUrl");
    issueJob(sockfd, "grep \"keyword\" /path/to/file1");
    issueJob(sockfd, "cat /path/to/file2");
    issueJob(sockfd, "./executable arg1 arg2");
    setConcurrency(sockfd, 4);
    stop(sockfd, "job_1");
    stop(sockfd, "job_15");
    poll(sockfd, "running");
    poll(sockfd, "queued");
    exitCommand(sockfd);

    // Κλείσιμο του socket
    close(sockfd);

    return 0;
}
