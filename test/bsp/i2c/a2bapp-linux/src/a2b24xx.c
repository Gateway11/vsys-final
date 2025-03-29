#include <pthread.h>
#include "a2bapp.h"

// echo "Hello, TCP" | nc 127.0.0.1 1234
// echo "Hello, UDP" | nc -u 127.0.0.1 1234
void* thread_loop(void *arg) {
#if 0
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(1234);

    for (int sock = socket(AF_INET, SOCK_DGRAM, 0), _m = 1; _m; _m--, sock >= 0 && close(sock)) {
        ipc_server_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
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
#endif
    return NULL;
}

int main(int argc, char* argv[]) {
    a2b_App_t gApp_Info;
    memset(&gApp_Info, 0, sizeof(gApp_Info));

    uint32_t nResult = 0;
    bool bRunFlag = true;

    gApp_Info.bFrstTimeDisc = A2B_TRUE;
    a2b_setup(&gApp_Info);

    pthread_t thread;
    pthread_create(&thread, NULL, thread_loop, NULL);
    pthread_detach(thread);

    while(1) {
        nResult = a2b_fault_monitor(&gApp_Info);
        if (nResult != 0) {
            bRunFlag = false;
        }

        a2b_stackTick(gApp_Info.ctx);
    }
    return 0;
}
