//
//  ipc_client.cpp
//
//  Created by 代祥 on 2023/11/27.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/types.h>

#define IPC_ENDPOINT 233

#define MBOX_RPROC_SAF 0
#define MBOX_RPROC_MP 2

#define AF_RPMSG      44
#define AF_RPMSG2      45
#define PF_RPMSG  AF_RPMSG

struct sockaddr_rpmsg {
    sa_family_t family;
    __u32 vproc_id;
    __u32 addr;
};

#define IPC_CLIENT_STR "Usage: ipc_client [domain: safety/mp/ap1/ap2/local] [message text] [send count]"

void ipc_write(int32_t sock, struct sockaddr* addr,
        socklen_t addrlen, const char* data, uint32_t size, uint32_t count) {
    char buf[128] = {0};
    int32_t ret = -1;

    if (connect(sock, addr, addrlen) != 0) {
        printf("%d, error: %s.\n", __LINE__, strerror(errno));
        return;
    }
    while (count) {
        ret = snprintf(buf, sizeof(buf), "%s; %03d", data, count--);
        ret = sendto(sock, buf, ret, 0, NULL, 0);

        usleep(1000 * 100);
        memset(buf, 0, sizeof(buf));
    }
    ret = sendto(sock, "exit", 4, 0, NULL, 0);
    (void)size;
}

void ipc_client_rpmsg(uint32_t domain, const char* data, uint32_t size, uint32_t count) {
    int32_t sock;
    struct sockaddr_rpmsg addr;

    sock = socket(PF_RPMSG, SOCK_SEQPACKET, 0); 
    addr.family = AF_RPMSG;
    if (sock < 0) {
        sock = socket(AF_RPMSG2, SOCK_SEQPACKET, 0);
        addr.family = AF_RPMSG2;
    }
    addr.vproc_id =  domain;
    addr.addr = IPC_ENDPOINT;

    ipc_write(sock, (struct sockaddr *)&addr, sizeof(addr), data, size, count);
    close(sock);
}

void ipc_client_socket(const char* ip_addr, const char* data, uint32_t size, uint32_t count) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(IPC_ENDPOINT);
    
    for (int sock = socket(AF_INET, SOCK_DGRAM, 0), _m = 1; _m; _m--, sock >= 0 && close(sock)) {
        ipc_write(sock, (struct sockaddr *)&addr, sizeof(addr), data, size, count);
    }
}

int main(int argc, const char * argv[]) {
    int sendCount = 1;
    if(argc < 3) {
        printf("%s, %s/%s\n", IPC_CLIENT_STR, __DATE__, __TIME__);
        return 0;
    }
    if(argc > 3)
        sendCount =  atoi(argv[3]);

    if (strcmp(argv[1], "safety") == 0) {
        ipc_client_rpmsg(MBOX_RPROC_SAF, argv[2], strlen(argv[2]), sendCount);
    } else if (strcmp(argv[1], "mp") == 0) {
        ipc_client_rpmsg(MBOX_RPROC_MP, argv[2], strlen(argv[2]), sendCount);
    } else if (strcmp(argv[1], "ap1") == 0) {
        ipc_client_socket("172.20.2.35", argv[2], strlen(argv[2]), sendCount);
    } else if (strcmp(argv[1], "ap2") == 0) {
        ipc_client_socket("172.20.2.36", argv[2], strlen(argv[2]), sendCount);
    } else if (strcmp(argv[1], "local") == 0) {
        ipc_client_socket("127.0.0.1", argv[2], strlen(argv[2]), sendCount);
    } else {
        printf("Domain is not supported!\n");
    }
    return 0;
}
