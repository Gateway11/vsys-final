//
//  main.cpp
//
//  Created by 代祥 on 2023/3/31.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <arpa/inet.h>

#include <unistd.h>
#include <string.h>

#define DP_CR5_SAF 0
#define AUDIO_MONITOR_EPT 125

/* user space needs this */
#ifndef AF_RPMSG
#define AF_RPMSG  44
#define PF_RPMSG  AF_RPMSG

struct sockaddr_rpmsg {
  __kernel_sa_family_t family;
  __u32 vproc_id;
  __u32 addr;
};

#define RPMSG_LOCALHOST ((__u32)~0UL)
#endif

int main(int argc, const char * argv[]) {
    struct sockaddr_rpmsg servaddr, cliaddr;

    memset(&servaddr, 0, sizeof(struct sockaddr_rpmsg));
    servaddr.family = AF_RPMSG;
    servaddr.vproc_id = DP_CR5_SAF;
    servaddr.addr = 132;//AUDIO_MONITOR_EPT;
    int32_t buf[128], ret;
    
    for (int s = socket(AF_INET, SOCK_STREAM, 0), _m = 1; _m; _m--, s > 0 && close(s)) {
        printf("connect ret=%d\n", connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)));
        for (uint32_t i = 0; i < 10; i++) {
			buf[1] = 23333;
			buf[0] = i;
			printf("write ret=%zd\n", write(s, (void *)buf, 8));
			ret = read(s, buf, sizeof(buf));
			printf("read  ret=%d, buf=%d, %d\n", ret, buf[0], buf[1]);
		}
    }
    return 0;
}
