
#include <pcap/pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void call_me(u_char *user, const struct pcap_pkthdr *pkthdr,
             const u_char *packetd_ptr) {
  printf("You just recieved a packet!\n");
  printf("ts : %ld caplen : %u len : %u\n", pkthdr->ts.tv_sec, pkthdr->caplen,
         pkthdr->len);

  char ip[32] = "";
  char buf[8] = "";
  for (u_int i = 0; i < 5; i++) {
    printf("%d.", packetd_ptr[i]);

    snprintf(buf, sizeof(buf), "%u", packetd_ptr[i]);
    if (i < 4) {
      strcat(buf, ".");
    }
    strcat(ip, buf);
  }

  printf("\n");

  for (u_int i = 0; i < 5; i++) {
    printf("%02x ", packetd_ptr[i]); // lecture seule
  }
  printf("\n");

  printf("ip: %s\n", ip);
}

int main(int argc, char const *argv[]) {
  char *device = "en0"; // remember to replace this with your device name
  char error_buffer[PCAP_ERRBUF_SIZE];
  int packets_count = 2;

  pcap_t *capdev = pcap_open_live(device, BUFSIZ, 0, 1000, error_buffer);

  if (capdev == NULL) {
    printf("ERR: pcap_open_live() %s\n", error_buffer);
    exit(1);
  }

  if (pcap_loop(capdev, packets_count, call_me, (u_char *)NULL)) {
    printf("ERR: pcap_loop() failed!\n");
    exit(1);
  }

  return 0;
}