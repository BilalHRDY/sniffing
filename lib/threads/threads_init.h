#ifndef THREADS_H
#define THREADS_H
#include "../sniffing/sniffing.h"
#include <pthread.h>

pthread_t *init_server_thread(void *data);
pthread_t *init_pcap_thread(void *data);
pthread_t *init_packet_queue_thread(void *data);

#endif
