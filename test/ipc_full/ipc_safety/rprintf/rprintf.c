//
//  rprintf.c
//
//  Created by 代祥 on 2023/5/22.
//

#include <lwip/sockets.h>
#include "rprintf.h"

static int32_t serverfd = -1;
static struct sockaddr_in servaddr;

int32_t __rprintf_sock_write(const char* data, uint32_t size) {
  //if (current_time() < 2000) return -1;
  if (serverfd < 0 && __sync_bool_compare_and_swap(&serverfd, -1, 0)) {
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = PP_HTONS(SERV_PORT);
    servaddr.sin_addr.s_addr = PP_HTONL(INADDR_LOOPBACK);
  
    if ((serverfd = socket(AF_INET, SOCK_DGRAM, /*IPPROTO_UDP*/0)) < 0) {
      dprintf(CRITICAL, "failed to create socket\n");
    }
  }
  while (!serverfd) thread_sleep(100);
  return sendto(serverfd, data, size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}
