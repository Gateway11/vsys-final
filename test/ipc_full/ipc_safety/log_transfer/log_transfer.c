//
//  log_transfer.c
//
//  Created by 薯条 on 2023/6/10.
//

#include <stdio.h>
#include <rpmsg_channel.h>
#include <rpmsg_service.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
//#include <../../drivers/kunlun/uart/dw_uart.h>
#include <CLI.h>

#define LOG_TRANSFER_ENDPOINT 2343

typedef struct log_transfer_msg {
  uint32_t msg_type;
  uint32_t msg_len;
  uint8_t data[];
} log_transfer_msg_t;

static uint32_t delay_ms = 10000;
static osEventFlagsId_t log_event;

int32_t log_transfer_event_handler(void *dmsg, int32_t len, unsigned long src, void *arg) 
{
  log_transfer_msg_t *msg = (log_transfer_msg_t *)dmsg;

  void *data = (void *)dmsg + sizeof(log_transfer_msg_t);
  printf("%s, %d\n", __func__, *(uint32_t *)data);

  if (msg->msg_type == 100) {
    delay_ms = *(uint32_t *)data;
    //event_signal(&log_event, false);
  } else {
    uart_control(msg->msg_type, data);
  }
  return 0;
}

static void log_transfer_service_entry(void *args) {
  struct rpmsg_channe *log_transfer_chan = rpmsg_channel_create(
          rpmsg_get_instance(1, MBOX_RPROC_AP1), "audio_monitor", LOG_TRANSFER_ENDPOINT);
  if (log_transfer_chan) {
    log_event = osEventFlagsNew(NULL);
    uart_control(1, &log_event);

    rpmsg_channel_start(log_transfer_chan, log_transfer_event_handler, NULL);
    int32_t max_payload = rpmsg_channel_max_payload(log_transfer_chan);

    uint8_t data[1024];//, data[log_transfer_chan->mtu + 1];
    while (true) {
      uint32_t ret = uart_rx_buf_read(data, max_payload);
      //data[ret] = '\0';
      //printf("mtu=%d, ret=%02d, data=%s\n", max_payload, ret, data);
      rpmsg_channel_send(log_transfer_chan, LOG_TRANSFER_ENDPOINT, (char *)data, ret, 100);
      if (ret != max_payload) {
        osEventFlagsWait(log_event, 1, osFlagsWaitAny, osWaitForever);
      }
    }
  } else {
    printf("failed to create ipcc channel %d\n", LOG_TRANSFER_ENDPOINT);
  }
}

void log_transfer_service_init(void)
{
  osThreadAttr_t attr = { .name = __func__, .stack_size = 2048, .priority = osPriorityNormal };
  osThreadNew(log_transfer_service_entry, NULL, &attr);
}
