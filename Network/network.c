#include "network.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>

int create_connection(char* node, char *service) {
    int server_socket;
    struct addrinfo *res = NULL;
    int gai_err;
    struct addrinfo hint = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
    };
    if ((gai_err = getaddrinfo(node, service, &hint, &res))) {
        fprintf(stderr, "gai error\n");
        return -1;
    }
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        if ((server_socket = socket(ai->ai_family, ai->ai_socktype, 0)) == -1) {
            perror("socket");
            continue;
        }
        if (connect(server_socket, ai->ai_addr, ai->ai_addrlen) < 0) {
            perror("connect");
            close(server_socket);
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return server_socket;
}

int create_listener(char* service) {
    struct addrinfo *res = NULL;
    int gai_err;
    struct addrinfo hint = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_flags = AI_PASSIVE
    };
    if ((gai_err = getaddrinfo(NULL, service, &hint, &res))) {
        return -1;
    }
    int sock = -1;
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sock < 0) {
            perror("socket");
            continue;
        }
        int tmp = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp)) < 0) {
            close(sock);
            sock = -1;
            continue;
        }
        if (bind(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            perror("bind");
            close(sock);
            sock = -1;
            continue;
        }
        if (listen(sock, SOMAXCONN) < 0) {
            perror("listen");
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return sock;
}

int send_message(int socket, char *buffer, size_t bytes, bool is_special) {
    char flag;
    if (is_special) {
        flag = '1';
    } else {
        flag = '0';
    }
    if (0 > write(socket, &flag, sizeof(flag))) {
        return -1;
    }
    if (0 > write(socket, &bytes, sizeof(bytes))) {
        return -1;
    }
    if (0 > write(socket, buffer, bytes)) {
        return -1;
    }
    return 0;
}