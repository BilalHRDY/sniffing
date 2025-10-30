#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SV_SOCK_PATH "tmp/sniffing_socket"
#define BUF_SIZE 1024

typedef struct {
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} input_buffer;

typedef struct {
  char *verb;
  int host_len;
  char **hostnames;
} command;

int has_null_terminator(const char *s) {
  int i = 0;
  while (1) {
    if (s[i] == '\0') {
      printf("has_null_terminator!\n");
      return 1;
    }
    printf("s[i]: %c\n", s[i]);
    i++;
  }
}

input_buffer *new_input_buffer() {
  input_buffer *input_buf = (input_buffer *)malloc(sizeof(input_buffer));
  input_buf->buffer = NULL;
  input_buf->buffer_length = 0;
  input_buf->input_length = 0;

  return input_buf;
}

void read_input(input_buffer *input_buf) {
  ssize_t bytes_read =
      getline(&(input_buf->buffer), &(input_buf->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  input_buf->input_length = bytes_read - 1;
  input_buf->buffer[bytes_read - 1] = 0;
}

int extract_words_from_input(input_buffer *input_buf, char *words[]) {
  int word_count = 0;
  size_t start_idx = 0;
  for (size_t i = 0; i <= input_buf->input_length; i++) {
    if (input_buf->buffer[i] == ' ' || i == input_buf->input_length) {
      size_t word_len = i - start_idx;
      if (word_len == 0) {
        start_idx = i + 1;
        continue;
      }

      words[word_count] = malloc((i - start_idx) * sizeof(char) + 1);
      if (!words[word_count]) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      strncpy(words[word_count], &(input_buf->buffer[start_idx]), word_len);
      words[word_count][word_len] = '\0';
      word_count++;
      start_idx = i + 1;
    }
  }
  for (size_t i = 0; i < word_count; i++) {
    printf("cmd: %s.\n", words[i]);
  }
  return word_count;
};

command *build_command(char *words[], int len) {
  command *cmd = malloc(sizeof(command));

  if (strcmp(words[0], "quit") == 0) {
    exit(EXIT_SUCCESS);
  } else if (strcmp(words[0], "add") == 0 || strcmp(words[0], "remove") == 0) {
    cmd->verb = strdup(words[0]);

    cmd->host_len = len;
    cmd->hostnames = words + 1;

    // printf("cmd->verb: %s\n", cmd->verb);
    // printf("cmd->hostnames[0]: %s\n", cmd->hostnames[0]);
    // printf("cmd->hostnames[1]: %s\n", cmd->hostnames[1]);
    return cmd;
  } else {
    fprintf(stderr, "Command '%s' not known!\n", words[0]);
    exit(EXIT_FAILURE);
  }
}

char *serialize_command(command *cmd) {
  size_t total_len = strlen(cmd->verb) + 1;
  printf("total_len: %zu\n", total_len);
  //   total_len += strlen(cmd->host_len) + 1;
  for (size_t i = 0; i < 2; i++) {
    total_len += strlen(cmd->hostnames[i]) + 1;
  }

  char *p = malloc(total_len);
  char *buf = p;
  strcpy(p, cmd->verb);
  p += strlen(cmd->verb) + 1;
  //   strcat(buf, cmd->host_len);
  for (size_t i = 0; i < 2; i++) {
    // strcat(buf, " ");
    strcpy(p, cmd->hostnames[i]);
    p += strlen(cmd->hostnames[i]) + 1;
  }

  printf("buf: %s\n", buf);
  printf("strlen(buf): %lu\n", strlen(buf));
  return buf;
}

char *handle_input(input_buffer *input_buf) {
  char *words[5];
  int word_count = extract_words_from_input(input_buf, words);
  command *cmd = build_command(words, word_count);
  return serialize_command(cmd);
}

int main(int argc, char *argv[]) {
  struct sockaddr_un addr;
  ssize_t numRead;
  input_buffer *input_buf = new_input_buffer();

  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  printf("Client socket fd = %d\n", sfd);

  if (sfd == -1) {
    perror("Error creating client socket");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("Error connecting to server socket");
    close(sfd);
    exit(EXIT_FAILURE);
  }

  //
  // Copy stdin to socket.
  //

  // Read at most BUF_SIZE bytes from STDIN into buf.
  while (1) {
    printf("sniffing >");
    read_input(input_buf);
    char *serialized_cmd = handle_input(input_buf);
    // Then, write those bytes from buf into the socket.

    if (write(sfd, serialized_cmd, strlen(serialized_cmd)) !=
        strlen(serialized_cmd)) {
      perror("Error writing to socket");
      close(sfd);
      exit(EXIT_FAILURE);
    }
  }

  if (numRead == -1) {
    perror("Error reading from stdin");
    close(sfd);
    exit(EXIT_FAILURE);
  }

  // Closes our socket; server sees EOF.
  exit(EXIT_SUCCESS);
}