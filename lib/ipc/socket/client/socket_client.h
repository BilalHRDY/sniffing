#include "../socket_common.h"
#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#define INPUT_MAX_SIZE 256

// TODO : input_handler_t doit il rester ici ? s

int init_socket(char *sock_path);

int write_and_read(int sfd, data_to_send_t *data_to_send,
                   data_received_t *data_received);

#endif
