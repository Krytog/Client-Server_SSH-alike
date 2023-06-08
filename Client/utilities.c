#include "utilities.h"
#include "../Queris/queris_types.h"
#include "../Queris/signals.h"
#include "../Network/network.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <sys/signal.h>

#define BUFFER_SIZE 4096

volatile sig_atomic_t k_socket = -1;

int get_command_type(char *arg) {
    if (!strcmp(arg, TYPE_SPAWN_STRING)) {
        return SPAWN;
    } else if (!strcmp(arg, TYPE_LIST_STRING)) {
        return LIST;
    } else if (!strcmp(arg, TYPE_ATTACH_STRING)) {
        return ATTACH;
    } else if (!strcmp(arg, TYPE_KILL_STRING)) {
        return KILL;
    } else if (!strcmp(arg, TYPE_PEEK_STRING)) {
        return PEEK;
    }
    return TYPE_FAILED;
}

void send_init_args(int socket, int argc, char **argv) {
    const int offset = 4;
    int args_to_send = argc - offset;
    write(socket, (void *)&args_to_send, sizeof(args_to_send));
    for (int i = 0; i < args_to_send; ++i) {
        size_t size = strlen(argv[i + offset]);
        write(socket, (void *)&size, sizeof(size));
        write(socket, argv[i + offset], size);
    }
}

void *read_worker(void *arg) {
    int socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = recv(socket, buffer, BUFFER_SIZE, 0)) > 0) {
        printf("%s", buffer);
        memset(buffer, '\0', bytes_read);
    }
    exit(EXIT_SUCCESS);
    return NULL;
}

void spawn_feature(int socket, int argc, char **argv) {
    k_socket = socket;
    int command_type = SPAWN;
    send_message(socket, (void *)&command_type, sizeof(command_type), true);
    send_init_args(socket, argc, argv);
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, read_worker, &socket);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        send_message(socket, buffer, bytes_read, false);
    }
    int signal = KEY_EOF;
    send_message(socket, (char *)&signal, sizeof(signal), true);
    pthread_join(thread_id, NULL);
}

void signal_handler(int signal) {
    if (signal == SIGINT) {
        int signal_send = KEY_SIGKILL;
        send_message(k_socket, (char *)&signal_send, sizeof(signal_send), true);
    }
}