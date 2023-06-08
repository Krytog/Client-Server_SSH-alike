#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/signal.h>
#include "../Network/network.h"
#include "../Queris/queris_types.h"
#include "utilities.h"

#define ERROR_MESSAGE_ARGS_NUM "Not enough arguments. Usage: ./Client Server_Ip Server_Port Command [Args...]"
#define ERROR_MESSAGE_CONNECTION_FAILED "Connection failed."
#define ERROR_MESSAGE_UNKNOWN_COMMAND "Unknown command:"

enum {
    ARGS_NUM = 4, ARG_IP = 1, ARG_PORT = 2, ARG_COMMAND = 3
};

int main(int argc, char **argv) {
    if (argc < ARGS_NUM) {
        printf("%s\n", ERROR_MESSAGE_ARGS_NUM);
    }
    struct sigaction sa = {
        .sa_handler = signal_handler,
        .sa_flags = SA_RESTART,
    };
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        return EXIT_FAILURE;
    }
    char *ip = argv[ARG_IP];
    char *port = argv[ARG_PORT];
    int socket = create_connection(ip, port);
    if (socket < 0) {
        printf("%s\n", ERROR_MESSAGE_CONNECTION_FAILED);
        exit(EXIT_FAILURE);
    }
    int command_type = get_command_type(argv[ARG_COMMAND]);
    if (command_type < 0) {
        printf("%s %s\n", ERROR_MESSAGE_UNKNOWN_COMMAND, argv[ARG_COMMAND]);
        exit(EXIT_FAILURE);
    }
    switch (command_type) {
        case SPAWN: {
            spawn_feature(socket, argc, argv);
            break;
        }
        default: {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}