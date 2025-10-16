
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
  struct Session **sessions = (struct Session **)context;

  char ip[16] = "";
  int pos = 0;
  int version;
  version = packetd_ptr[14] >> 4;

  // printf("version : %u\n", version);
  uint32_t ip_num = (packetd_ptr[30] << 24) | (packetd_ptr[31] << 16) |
                    (packetd_ptr[32] << 8) | (packetd_ptr[33]);

  printf("ip number recu : %u\n", ip_num);
  printf("ip number enregistré : %u\n", sessions[0]->ip);
  if (version == 4) {

    for (u_int i = 0; i < 4; i++) {
      pos += snprintf(ip + pos, sizeof(ip) - pos, "%u", packetd_ptr[30 + i]);

      // Ajoute un '.' après chaque octet sauf le dernier
      if (i < 3) {
        pos += snprintf(ip + pos, sizeof(ip) - pos, ".");
      }
    }
  }
  if (sessions[0]->ip == 0) {

    sessions[0]->ip = ip_num;
    sessions[0]->firstVisit = pkthdr->ts.tv_sec;
  }
  printf("ip : %s", ip);
  // if (sessions[0]->ip == )
  printf("\n");
}

int main(int argc, char const *argv[]) {
  char *device = "en0"; // remember to replace this with your device name
  char error_buffer[PCAP_ERRBUF_SIZE];
  int packets_count = 2;
  char *filter = "tcp and (dst host 151.101.2.217 or dst host 151.101.130.217 "
                 "or dst host 151.101.66.217 "
                 "or dst host 151.101.194.217)";

  struct bpf_program fp;

  struct Session *sessions[3];
  sessions[0] = malloc(sizeof(Session));
  struct in_addr addr;
  char *ip = "151.101.2.217";

  if (inet_aton(ip, &addr) == 0) {
    fprintf(stderr, "Invalid address\n");
    exit(EXIT_FAILURE);
  }
  printf("test ip : %u\n", addr.s_addr);
  sessions[0]->ip = addr.s_addr;

  pcap_t *handle;
  bpf_u_int32 net, mask;

  pcap_lookupnet("en0", &net, &mask, error_buffer);
  struct in_addr ip_addr;
  ip_addr.s_addr = net;
  struct in_addr mask_addr;
  mask_addr.s_addr = mask;
  printf("Network: %s\n", inet_ntoa(ip_addr));
  printf("Mask: %s\n", inet_ntoa(mask_addr));

  handle = pcap_open_live(device, BUFSIZ, 0, 1000, error_buffer);

  if (handle == NULL) {
    fprintf(stderr, "Erreur: %s\n", error_buffer);

    exit(1);
  }

  if (pcap_compile(handle, &fp, filter, 1, mask)) {
    fprintf(stderr, "Erreur pcap_compile: %s\n", pcap_geterr(handle));
    exit(1);
  }
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(handle));
    exit(1);
  }
  if (pcap_loop(handle, packets_count, call_me, (u_char *)sessions)) {
    fprintf(stderr, "Erreur pcap_setfilter: %s\n", pcap_geterr(handle));
    exit(1);
  }

  return 0;
}