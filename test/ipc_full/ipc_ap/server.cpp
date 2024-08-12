//
//  ipc_server.cpp
//
//  Created by 代祥 on 2023/11/27.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/types.h>

#define MBOX_RPROC_SAF 0
#define MBOX_RPROC_MP 2

#define AF_RPMSG      44
#define AF_RPMSG2      45
#define PF_RPMSG  AF_RPMSG

#define IPC_ENDPOINT 233

struct sockaddr_rpmsg {
    sa_family_t family;
    __u32 vproc_id;
    __u32 addr;
};

#define IPC_SERVER_STR "ipc_server [safety mp ap1 ap2]"

void ipc_server_bind(int32_t sock, struct sockaddr* addr, socklen_t addrlen) {
    char buf[128] = {0};
    int32_t ret;

    if (bind(sock, addr, addrlen) != 0) {
        printf("%d, error: %s.\n", __LINE__, strerror(errno));
        return;
    }
    usleep(1000 * 25);
    
    while (strncmp(buf, "exit", 4)) {
        memset(buf, 0, sizeof(buf));
        ret = recvfrom(sock, buf, sizeof(buf), 0, NULL, 0);
        printf("%s\n", buf);
    }
}

void ipc_server_rpmsg(uint32_t domain) {
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

    ipc_server_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    close(sock);
}

void ipc_server_socket() {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(IPC_ENDPOINT);
    
    for (int sock = socket(AF_INET, SOCK_DGRAM, 0), _m = 1; _m; _m--, sock >= 0 && close(sock)) {
        ipc_server_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    }
}

#ifdef __ANDROID_NDK__
int main(int argc, const char * argv[]) {
    if(argc < 2) {
        printf("%s, %s/%s\n", IPC_SERVER_STR, __DATE__, __TIME__);
        return 0;
    }
    if (strcmp(argv[1], "safety") == 0) {
        ipc_server_rpmsg(MBOX_RPROC_SAF);
    } else if (strcmp(argv[1], "mp") == 0) {
        ipc_server_rpmsg(MBOX_RPROC_MP);
    } else if (strcmp(argv[1], "ap1") == 0) {
        ipc_server_socket();
    } else if (strcmp(argv[1], "ap2") == 0) {
        ipc_server_socket();
    } else {
        printf("Domain is not supported!\n");
    }
    return 0;
}
#endif
