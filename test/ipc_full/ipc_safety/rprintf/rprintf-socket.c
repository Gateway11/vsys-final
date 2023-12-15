//
//  rprintf-socket.c
//
//  Created by 代祥 on 2023/5/22.
//

#include <app.h>
#include <thread.h>
#include <lwip/sockets.h>

#include "rprintf.h"
#include "rprintf-ipcc.h"

#define SERV_PORT 2333

static int32_t serverfd = -1;
static struct sockaddr_in servaddr;

static void pre_rprintf(void) {
  if (serverfd < 0 && __sync_bool_compare_and_swap(&serverfd, -1, 0)) {
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = PP_HTONS(SERV_PORT);
    servaddr.sin_addr.s_addr = PP_HTONL(INADDR_LOOPBACK);
  
    if ((serverfd = socket(AF_INET, SOCK_DGRAM, /*IPPROTO_UDP*/0)) < 0) {
      dprintf(CRITICAL, "failed to create socket\n");
    }
#if 0
    struct timeval tv = { .tv_sec = 0, .tv_usec = 3000/* ms */ * 100 };
    setsockopt(serverfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
  }
  while (!serverfd) thread_sleep(100);
}

int32_t __rprintf_sock_write(const char* data, uint32_t size) {
  pre_rprintf();
  return sendto(serverfd, data, size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

static int32_t rprintf_hub(void *arg) {
  rprintf_ipcc_init();

  int32_t listenfd, ret, recv_data[RECV_DATA >> 2];
  struct sockaddr_in recvaddr;

  memset(&recvaddr, 0, sizeof(recvaddr));
  recvaddr.sin_family = AF_INET;
  recvaddr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
  recvaddr.sin_port = PP_HTONS(SERV_PORT);

  if ((listenfd = socket(AF_INET, SOCK_DGRAM, /*IPPROTO_UDP*/0)) >= 0) {

    //setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, (uint32_t[]){4*1024*1024}, sizeof(uint32_t));

    if (bind(listenfd, (struct sockaddr *)&recvaddr, sizeof(recvaddr)) != 0) {
      printf("%s: %d errno=%d, %s\n", __func__, __LINE__, errno, strerror(errno));
    }
    while (true) {
      ret = recvfrom(listenfd, recv_data, sizeof(recv_data), 0, NULL, 0);

      //((char *)recv_data)[ret] = '\0';
      //printf("ret=%d, data=%s\n", ret, recv_data);
      ret = rprintf_ipcc_write((char *)recv_data, ret);
    }
  }
  return 0;
}

void rprintf_service_init(const struct app_descriptor *app) {
  thread_t *thread = thread_create(__func__, rprintf_hub, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
  thread_detach_and_resume(thread);
}

APP_START(rprintf).init = rprintf_service_init, .flags = 0, APP_END
