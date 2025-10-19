#include "lib/sniffing.h"
#include "lib/utils/hashmap.h"
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
struct Session {
  time_t firstVisit;
  time_t lastVisit;
  u_int ip;
} Session;

void call_me(u_char *context, const struct pcap_pkthdr *pkthdr,
             const u_char *packetd_ptr) {
  printf("You just recieved a packet!\n");
  printf("secondes : %ld caplen : %u len : %u\n", pkthdr->ts.tv_sec,
         pkthdr->caplen, pkthdr->len);
  // struct Session **sessions = (struct Session **)context;

  char ip[16] = "";
  int pos = 0;
  int version;
  version = packetd_ptr[14] >> 4;

  // printf("version : %u\n", version);
  uint32_t ip_num = (packetd_ptr[30] << 24) | (packetd_ptr[31] << 16) |
                    (packetd_ptr[32] << 8) | (packetd_ptr[33]);

  printf("ip number recu : %u\n", ip_num);
  // printf("ip number enregistré : %u\n", sessions[0]->ip);

  // if (sessions[0]->ip == 0) {

  //   sessions[0]->ip = ip_num;
  //   sessions[0]->firstVisit = pkthdr->ts.tv_sec;
  // }
  printf("ip : %s", ip);
  // if (sessions[0]->ip == )
  printf("\n");
}

int main(int argc, char const *argv[]) {
  char *device = "en0";
  char error_buffer[PCAP_ERRBUF_SIZE];
  int packets_count = 2;

  int status;
  char ipstr[INET6_ADDRSTRLEN], ipver;
  char *filter;
  ht *table = ht_create();

  if (argc < 2) {
    fprintf(stderr, "You have to specified hostname!\n");
    exit(EXIT_FAILURE);
  }
  init_hosts_table_and_filter(table, argv + 1, &filter);

  print_table(table);
  struct bpf_program fp;

  struct Session *sessions[3];
  sessions[0] = malloc(sizeof(Session));

  pcap_t *handle;
  bpf_u_int32 net, mask;

  pcap_lookupnet("en0", &net, &mask, error_buffer);

  handle = pcap_open_live(device, BUFSIZ, 0, 1000, error_buffer);

  if (handle == NULL) {
    fprintf(stderr, "Erreur: %s\n", error_buffer);
    exit(EXIT_FAILURE);
  }

  if (pcap_compile(handle, &fp, filter, 1, mask)) {
    fprintf(stderr, "Erreur pcap_compile: %s\n", pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }
  if (pcap_loop(handle, packets_count, call_me, (u_char *)sessions)) {
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }

  return 0;
}