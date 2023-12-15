#include <stdio.h>
#include <string.h>
#include <rpmsg_channel.h>
#include <rpmsg_service.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>

#include "CLI_type.h"
#ifdef CONFIG_MP
#define IPC_SERVER_STR "ipc_server [safety ap1 ap2]"
#else
#define IPC_SERVER_STR "ipc_server [mp ap1 ap2]"
#endif
#define IPC_ENDPOINT 233

static void ipc_server_rpmsg(uint8_t domain) {
    uint32_t src, len;
    char buf[128] = {0};

    struct rpmsg_channe *server_chan = rpmsg_channel_create(
            rpmsg_get_instance(1, domain), "audio_monitor", IPC_ENDPOINT);
    rpmsg_channel_start(server_chan, NULL, NULL);

    while (strncmp(buf, "exit", 4)) {
        memset(buf, 0, sizeof(buf));
        rpmsg_channel_recvfrom(server_chan, &src, buf, sizeof(buf), (int32_t*)&len, INT_MAX);

        printf("%s\n", buf);
        //rpmsg_channel_send(server_chan, IPC_ENDPOINT, buf, len, 100);
    }
    rpmsg_channel_destroy(server_chan);
}

static int32_t ipc_server(int argc, char *argv[]) {
    if (argc < 1) {
        printf("%s, %s/%s\n", IPC_SERVER_STR, __DATE__, __TIME__);
        return 0;
    }
    if (strcmp(argv[0], "ap1") == 0) {
        ipc_server_rpmsg(MBOX_RPROC_AP1);
#ifdef CONFIG_MP
    } else if (strcmp(argv[0], "safety") == 0) {
        ipc_server_rpmsg(MBOX_RPROC_SAF);
#else
    } else if (strcmp(argv[0], "mp") == 0) {
        ipc_server_rpmsg(MBOX_RPROC_MP);
#endif
    } else if (strcmp(argv[0], "ap2") == 0) {
        ipc_server_rpmsg(MBOX_RPROC_AP2);
    } else {
        printf("Domain is not supported!\n");
    }
    return 0;
}

CLI_CMD("ipc_server", IPC_SERVER_STR, ipc_server);
