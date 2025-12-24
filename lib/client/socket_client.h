#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

// TODO: a factoriser avec server
#define BUF_SIZE 1024
#define INPUT_MAX_SIZE 256

// void init_socket();

int start_client(char *sock_path, input_handler_t input_handler,
                 handle_response_t handle_response);

typedef void (*input_handler_t)(unsigned char **data_to_send,
                                ssize_t *data_to_send_len);

typedef void (*handle_response_t)(char buf[BUF_SIZE],
                                  ssize_t *data_to_send_len);

#endif
