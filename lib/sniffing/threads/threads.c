#include "../../ipc/server_loop/server_loop.h"
#include "../sniffing.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_t *init_server_thread(void *ctx) {

  pthread_t *server_thread = malloc(sizeof(pthread_t));

  int server_thread_res = pthread_create(server_thread, NULL, server_loop, ctx);

  if (server_thread_res) {
    fprintf(stderr, "error while creating server thread!\n");
    exit(EXIT_FAILURE);
  }
  return server_thread;
}

// PCAP + cache + filter
void *pcap_runner_thread(context_t *ctx) {
  //   context_t *ctx = (context_t *)data;

  // printf("thread lock\n");
  pthread_mutex_lock(&ctx->mutex2);
  // printf("thread in lock\n");

  while (1) {

    while (ctx->paused) {
      printf("pcap is paused\n");
      pthread_cond_wait(&ctx->condition2, &ctx->mutex2);
    }
    printf("pcap is starting....\n");
    while (!ctx->paused) {

      pthread_mutex_unlock(&ctx->mutex2);
      pcap_dispatch(ctx->handle, -1, packet_handler, (u_char *)ctx);
      pthread_mutex_lock(&ctx->mutex2);
    }
  }

  pthread_mutex_unlock(&ctx->mutex2);
  // printf("end of thread\n");
  return NULL;
}