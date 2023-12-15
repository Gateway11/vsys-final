//
//  rprintf-ipcc.c
//
//  Created by 代祥 on 2023/5/22.
//

#include <dcf.h>
#include "rprintf.h"
#include "rprintf-ipcc.h"

#define RPRINTF_ENDPOINT 2333

static struct ipcc_channel *rprintf_chan = NULL;

static int32_t rprintf_task (void * para) {
  uint8_t *data = malloc(rprintf_chan->mtu);
  unsigned long src;

  while(true) {
    int32_t len = rprintf_chan->mtu;
    int32_t ret = ipcc_channel_recvfrom(rprintf_chan, &src, (char *)data, &len, INFINITE_TIME);
  }
  free(data);
  return 0;
}

int32_t rprintf_ipcc_init (void) {
  struct ipcc_device *rprintf_ipcc_dev = ipcc_getdevice(IPCC_RRPOC_AP1);

  rprintf_chan = ipcc_channel_create(rprintf_ipcc_dev, RPRINTF_ENDPOINT, "audio_monitor", true);
  if (rprintf_chan) {
    ipcc_channel_set_mtu(rprintf_chan, IPCC_MB_MTU);
    ipcc_channel_start(rprintf_chan, NULL);

    //thread_t *thread = thread_create(__func__, rprintf_task, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    //thread_detach_and_resume(thread);
  } else {
    dprintf(CRITICAL, "failed to create ipcc channel %d\n", RPRINTF_ENDPOINT);
    return -1;
  }
  return 0;
}

int32_t rprintf_ipcc_write(char* data, uint32_t size) {
  return ipcc_channel_sendto(rprintf_chan, RPRINTF_ENDPOINT, data, 
      size > rprintf_chan->mtu ? rprintf_chan->mtu : size, 100);
}
