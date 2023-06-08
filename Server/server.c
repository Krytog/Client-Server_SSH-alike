#include "../Network/network.h"
#include "../Queris/queris_types.h"
#include "utilities.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define ERROR_MESSAGE_ARGS_NUM "Wrong number of arguments. Usage: ./Server Port"

enum {
    ARGS_NUM = 2, ARG_PORT = 1
};

int main(int argc, char **argv) {
    if (argc != ARGS_NUM) {
        printf("%s\n", ERROR_MESSAGE_ARGS_NUM);
        exit(EXIT_FAILURE);
    }
    go_daemon();
    int listener = create_listener(argv[ARG_PORT]);
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
        pid_t pid = fork();
        if (!pid) {
            char flag = '2';
            read(client, &flag, sizeof(flag));
            if (flag != '1') {
                exit(EXIT_FAILURE);
            }
            size_t size;
            read(client, &size, sizeof(size));
            int query_type;
            read(client, &query_type, size);
            switch(query_type) {
                case SPAWN: {
                    spawn_feature(listener, client);
                    break;
                }
            }
            exit(EXIT_SUCCESS);
        } else {
            close(client);
        }
    }
    return EXIT_SUCCESS;
}