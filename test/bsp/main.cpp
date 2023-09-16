//
//  main.cpp
//  audio_tools
//
//  Created by 代祥 on 2023/3/31.
//

#include <sys/socket.h>
#include <fcntl.h> //open
//#include <sys/stat.h>

#include <fstream>
#include <thread>
#include <vector>

#define DP_CR5_SAF 0
#define AUDIO_MONITOR_EPT 2333

#if !defined(AF_RPMSG) && !defined(__KERNEL__)
/* user space needs this */
#define AF_RPMSG 44
#define PF_RPMSG AF_RPMSG

struct sockaddr_rpmsg {
  //__kernel_sa_family_t family;
  sa_family_t family;
  __be32 vproc_id;
  __be32 addr;
};

#define RPMSG_LOCALHOST ((__u32)~0UL)
#else
#include <uapi/linux/rpmsg_socket.h> //struct sockaddr_rpmsg
#endif

#if defined(__ANDROID__) || defined(ANDROID)
#include <unistd.h> //read/write
#include <arpa/inet.h> //htonl/inet_pton
#include <android/log.h>
#define LOG_TAG "ipc_test"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#elif !defined(__KERNEL__) //linux application
#include <linux/types.h>
#include <linux/in.h>
#include <linux/inet.h> //htonl/inet_pton
#include <netinet/in.h>
#endif
//#if __STDC_VERSION__ ==  201112L

int output = open("/data/test.txt", O_WRONLY|O_CREAT|O_APPEND, S_IWUSR|S_IRGRP|S_IROTH);
int main(int argc, const char * argv[]) {
    //https://www.demo2s.com/c/c-sock-socket-af-rpmsg-sock-seqpacket-0.html
    //SemiDrive_X9_音频应用指南_Rev1.1.pdf -43
#if 0
    int32_t sock, ret, data[1024];

    struct sockaddr_rpmsg servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.family = AF_RPMSG;
    servaddr.vproc_id = DP_CR5_SAF;
    servaddr.addr = AUDIO_MONITOR_EPT;

    if ((sock = socket(PF_RPMSG, SOCK_SEQPACKET, 0)) != -1) {
        struct timeval tv = { .tv_usec = 300/* ms */ * 1000 };
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
            ALOGE("Set rcv timeo failed");
        if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
            ALOGE("Set snd timeo failed");
        ret = connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));
        while (true) {
            ret = recvfrom(sock, data, sizeof(data), 0, NULL, 0);
            ret = sendto(sock, &ret, sizeof(ret), 0, NULL, 0);
        }
    } else {
        ALOGE("ipcc init failed, errno: %d (%s)", sock, strerror(errno));
    }
#else
    struct sockaddr_in servaddr, recvaddr, cliaddr;
    int32_t listenfd, clientfd, connfd, sock, ret, len;
    socklen_t clilen = sizeof(cliaddr);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    //servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(2333);

    char buf[1024], str[64];

    for (int32_t sock = socket(AF_INET, SOCK_STREAM, 0), _m = 1; _m; _m--, sock > 0 && close(sock)) {
    //if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) != -1) {
        if (bind(sock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) == 0) {
            listen(sock, 20);
            while (true) {
                connfd = accept(sock, (struct sockaddr *)&cliaddr, &clilen);
                //ret = recvfrom(s, &buf, 1024, 0,(struct sockaddr *)&cliaddr, &clilen);
                //ret = sendto(s, &ret, sizeof(ret), 0, (sockaddr *)&cliaddr, sizeof(sockaddr_in));
                printf("received from %s at PORT %d\n", inet_ntop(AF_INET,
                            &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
                while (true) {
                    read(connfd, buf, len);        //socket read
                    //ret = write(output, buf, n);    //file IO
                    write(connfd, &ret, 4);        //socket write
                }
            }
        }
    }
    std::vector<std::thread> threads;
    threads.emplace_back([&]{

    });
    std::for_each(threads.begin(), threads.end(), [](std::thread& thread){thread.join();});
#endif
#if 0
    uint32_t sample_rate = 44100;
    uint32_t num_channels = 2;
    uint32_t num_samples = sample_rate * num_channels * sizeof(short) / 100;
    
    std::vector<std::thread> threads;
    
    paths.push_back("/Users/daixiang/Music/test.wav");
    paths.push_back("/Users/daixiang/Music/test.wav");
    paths.push_back("/Users/daixiang/Music/test.wav");
    
    for (uint32_t i = 0; i < 3; i++) {
        threads.emplace_back([i, num_samples]{

            char *data(new char[num_samples]);
            
            std::ifstream input_stream(paths[i].c_str(), std::ios::in | std::ios::binary);
            while (input_stream.good()) {
                input_stream.read(data, num_samples);
                
                easy_write("tddd", i, 0, 0x30, 44100, 2, 16, (char *)data, num_samples, false);
            }
        });
    }
    std::for_each(threads.begin(), threads.end(), [](std::thread& thread){thread.join();});
#endif
//    std::cout << "Hello, World!\n";
    return 0;
}
