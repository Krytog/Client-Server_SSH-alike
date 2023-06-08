int get_command_type(char *arg);
void send_init_args(int socket, int argc, char **argv);
void *read_worker(void* arg);

void spawn_feature(int socket, int argc, char **argv);

void signal_handler(int signal);