
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 25




void write_to_pipe(int write_fd, const char* filename) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    size_t nread;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while ((nread = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        if (write(write_fd, buffer, nread) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    fclose(fp);
}

void read_from_pipe(int read_fd) {
    char buffer[BUFFER_SIZE];
    int word_count = 0;
    ssize_t nread;
    int in_word = 0;

    while ((nread = read(read_fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < nread; i++) {
            if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t') {
                in_word = 0;
            } else {
            	if (!in_word) {
            		word_count++;
            		in_word = 1;
            	}
            }
        }
    }

    printf("Number of words: %d\n", word_count);
}

int main() {
    int pipefd[2];
    pid_t cpid;

    char filename[] = "test.txt";

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) { // Child reads from pipe
        close(pipefd[1]);          // Close unused write end
        read_from_pipe(pipefd[0]);
        close(pipefd[0]);
        _exit(EXIT_SUCCESS);

    } else { // Parent writes to pipe 
        close(pipefd[0]);          // Close unused read end
        write_to_pipe(pipefd[1], filename);
        close(pipefd[1]);          // Reader will see EOF
        wait(NULL);                // Wait for child
        exit(EXIT_SUCCESS);
    }
}
