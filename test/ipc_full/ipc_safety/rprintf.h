//
//  rprintf.h
//
//  Created by 薯条 on 2023/5/22.
//

#ifndef __RPRINTF_H__
#define __RPRINTF_H__

#define MAX_DATA 256
#define RPRINTF_ENDPOINT 2333

#ifdef CONFIG_LWIP
#include <assert.h>
#include <lwip/sockets.h>

static int32_t serverfd = -1;
static struct sockaddr_in servaddr;
static int32_t __rprintf_ipc_write(const char* data, uint32_t size) {
  if (serverfd < 0 && __sync_bool_compare_and_swap(&serverfd, -1, 0)) {
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = PP_HTONS(RPRINTF_ENDPOINT);
    servaddr.sin_addr.s_addr = inet_addr("172.20.2.35");

    assert((serverfd = socket(AF_INET, SOCK_DGRAM, /*IPPROTO_UDP*/0)) != -1);
  }
  while (!serverfd) osDelay(1);
  return sendto(serverfd, data, size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
}
#else
#include <rpmsg_channel.h>
#include <rpmsg_service.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>

static struct rpmsg_channe *rprintf_chan = NULL;
static int32_t max_payload = -1;
static int32_t __rprintf_ipc_write(const char* data, uint32_t size) {
  if (max_payload < 0 && __sync_bool_compare_and_swap(&max_payload, -1, 0)) {
    rprintf_chan = rpmsg_channel_create(
            rpmsg_get_instance(1, MBOX_RPROC_AP1), "audio_monitor", RPRINTF_ENDPOINT);
    rpmsg_channel_start(rprintf_chan, NULL, NULL);

    //osDelay(50);
    max_payload = rpmsg_channel_max_payload(rprintf_chan);
  }
  while (!max_payload) osDelay(1);
  return rpmsg_channel_send(rprintf_chan, RPRINTF_ENDPOINT, (char *)data, size, 100);
}
#endif

#define rprintf(format, ...) ({                                                      \
  char __str[MAX_DATA];                                                              \
  __rprintf_ipc_write(__str, snprintf(__str, sizeof(__str), format, ##__VA_ARGS__)); \
})

#endif /* __RPRINTF_H__ */
