#include <stddef.h>
#include <stdbool.h>

int create_connection(char *node, char *service);
int create_listener(char *service);
int send_message(int socket, char *buffer, size_t bytes, bool is_special);