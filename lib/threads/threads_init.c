#include "../ipc/server_loop/server_loop.h"
#include "../sniffing/sniffing.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Faire une unique fonction
pthread_t *init_server_thread(void *data) {
  pthread_t *server_thread = malloc(sizeof(pthread_t));
  int rc = pthread_create(server_thread, NULL, server_loop, data);

  if (rc) {
    fprintf(stderr, "error while creating server thread!\n");
    exit(EXIT_FAILURE);
  }
  return server_thread;
}

pthread_t *init_pcap_thread(void *data) {
  pthread_t *pcap_thread = malloc(sizeof(pthread_t));
  int rc = pthread_create(pcap_thread, NULL, pcap_runner_loop, data);

  if (rc) {
    fprintf(stderr, "error while creating pcap thread!\n");
    exit(EXIT_FAILURE);
  }
  return pcap_thread;
}

pthread_t *init_packet_queue_thread(void *data) {
  pthread_t *packet_queue_thread = malloc(sizeof(pthread_t));
  int rc = pthread_create(packet_queue_thread, NULL, packet_queue_loop, data);

  if (rc) {
    fprintf(stderr, "error while creating packet queue thread!\n");
    exit(EXIT_FAILURE);
  }
  return packet_queue_thread;
}
