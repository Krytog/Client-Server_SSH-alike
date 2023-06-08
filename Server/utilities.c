#include "../Queris/signals.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 4096

void go_daemon() {
    if (fork()) {
        exit(EXIT_SUCCESS);
    }
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
}

int receive_args_from_client(int client, char ***argv) {
    int argc;
    recv(client, &argc, sizeof(argc), 0);
    *argv = calloc(argc, sizeof(**argv));
    for (int i = 0; i < argc; ++i) {
        size_t size;
        if (recv(client, &size, sizeof(size), 0) <= 0) {
            free(*argv);
            return -1;
        }
        (*argv)[i] = calloc(size, sizeof(***argv));
        if (recv(client, (*argv)[i], size, 0) <= 0) {
            free(*argv);
            return -1;
        }
    }
    return 0;
}

void blocking_reading(int client, int write_to, pid_t resolver) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, '\0', BUFFER_SIZE);
        char flag;
        recv(client, &flag, sizeof(flag), 0);
        if (flag == '1') {
            size_t bytes;
            int message;
            recv(client, &bytes, sizeof(bytes), 0);
            recv(client, &message, bytes, 0);
            switch (message) {
                case KEY_EOF: {
                    close(write_to);
                    break;
                }
                case KEY_SIGKILL: {
                    kill(resolver, SIGKILL);
                    break;
                }
            }
        } else {
            size_t bytes;
            recv(client, &bytes, sizeof(bytes), 0);
            recv(client, buffer, bytes, 0);
            write(write_to, buffer, bytes);
        }
    }
}

void *wait_worker(void *arg) {
    pid_t pid = *(pid_t *)arg;
    waitpid(pid, NULL, 0);
    return NULL;
}

void spawn_feature(int socket, int client) {
    char **argv;
    if (receive_args_from_client(client, &argv)) {
        exit(EXIT_FAILURE);
    }
    int pipe_fd[2];
    if (pipe(pipe_fd) < 0) {
        exit(EXIT_FAILURE);
    }

    pid_t resolver = fork();

    if (resolver > 0) {
        close(pipe_fd[0]);
        pthread_t waiter;
        pthread_create(&waiter, NULL, wait_worker, &resolver);
        blocking_reading(client, pipe_fd[1], resolver);
        close(client);
        close(socket);
        close(pipe_fd[1]);
        pthread_kill(waiter, SIGKILL);
        pthread_join(waiter, NULL);
        exit(0);
    } else {
        dup2(pipe_fd[0], STDIN_FILENO);
        dup2(client, STDOUT_FILENO);
        close(client);
        close(socket);
        close(pipe_fd[1]);
        execvp(argv[0], argv);
        exit(EXIT_FAILURE);
    }
}