#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpmsg_channel.h>
#include <rpmsg_service.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>

#include "CLI_type.h"
#ifdef CONFIG_MP
#define IPC_CLIENT_STR "ipc_client [safety ap1 ap2][text][count]"
#else
#define IPC_CLIENT_STR "ipc_client [mp ap1 ap2][text][count]"
#endif
#define IPC_ENDPOINT 233

static void ipc_write(uint8_t domain, char* data, uint32_t size, uint32_t count) {
    char buf[128] = {0};
    uint32_t ret;

    struct rpmsg_channe *client_chan = rpmsg_channel_create(
            rpmsg_get_instance(1, domain), "audio_monitor", IPC_ENDPOINT);
    rpmsg_channel_start(client_chan, NULL, NULL);
    //int32_t max_payload = rpmsg_channel_max_payload(client_chan);
    osDelay(25);

    while (count) {
        ret = snprintf(buf, sizeof(buf), "%s %03d", data, count--);
        rpmsg_channel_send(client_chan, IPC_ENDPOINT, buf, ret, 100);

        osDelay(100);
        memset(buf, 0, sizeof(buf));
    }
    rpmsg_channel_send(client_chan, IPC_ENDPOINT, "exit", 4, 100);
    osDelay(50);
    rpmsg_channel_destroy(client_chan);
}

static int32_t ipc_client(int argc, char *argv[]) {
    if (argc < 3) {
        printf("%s, %s/%s\n", IPC_CLIENT_STR, __DATE__, __TIME__);
        return 0;
    }
    if (strcmp(argv[0], "ap1") == 0) {
        ipc_write(MBOX_RPROC_AP1, argv[1], strlen(argv[1]), atoi(argv[2]));
#ifdef CONFIG_MP
    } else if (strcmp(argv[0], "safety") == 0) {
        ipc_write(MBOX_RPROC_SAF, argv[1], strlen(argv[1]), atoi(argv[2]));
#else
    } else if (strcmp(argv[0], "mp") == 0) {
        ipc_write(MBOX_RPROC_MP, argv[1], strlen(argv[1]), atoi(argv[2]));
#endif
    } else if (strcmp(argv[0], "ap2") == 0) {
        ipc_write(MBOX_RPROC_AP2, argv[1], strlen(argv[1]), atoi(argv[2]));
    } else {
        printf("Domain is not supported!\n");
    }
    return 0;
}

CLI_CMD("ipc_client", IPC_CLIENT_STR, ipc_client);
