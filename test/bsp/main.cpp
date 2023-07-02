//
//  main.cpp
//  audio_tools
//
//  Created by 代祥 on 2023/3/31.
//

#include <fstream>
#include <thread>
#include <vector>

#include <sys/socket.h>
#include <stdio.h>
//#include <unistd.h>

//#include <linux/types.h>
//#include <linux/in.h>
//#include <netinet/in.h>
//#include <linux/inet.h> //htonl/inet_pton
//#include <arpa/inet.h> //htonl/inet_pton

#include <fcntl.h>
#include <sys/stat.h>

#define DP_CR5_SAF 0
#define AUDIO_MONITOR_EPT 2333

/* user space needs this */
#if !defined(AF_RPMSG) && !defined(__KERNEL__)
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
#include <linux/inet.h>
#include <uapi/linux/rpmsg_socket.h>
#endif

#include <android/log.h>
#define LOG_TAG "ipc_test"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#if __has_include(<android/log.h>)
#warning "ddddddddddddddd %s"__cplusplus
#endif
//#if __STDC_VERSION__ ==  201112L

int output = open("/data/test.txt", O_WRONLY|O_CREAT|O_APPEND, S_IWUSR|S_IRGRP|S_IROTH);
int main(int argc, const char * argv[]) {
    //https://www.demo2s.com/c/c-sock-socket-af-rpmsg-sock-seqpacket-0.html
    //SemiDrive_X9_音频应用指南_Rev1.1.pdf -43
#if 1
    int32_t serverfd, ret;
    uint32_t* data[1024], len;

    struct sockaddr_rpmsg servaddr, dst_addr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.family = AF_RPMSG;
    servaddr.vproc_id = DP_CR5_SAF;
    servaddr.addr = AUDIO_MONITOR_EPT;

    if ((serverfd = socket(PF_RPMSG, SOCK_SEQPACKET, 0)) != -1) {
        struct timeval tv = { .tv_usec = 300/* ms */ * 1000 };
        if (setsockopt(serverfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
            ALOGE("Set rcv timeo failed");
        if (setsockopt(serverfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
            ALOGE("Set snd timeo failed");
        ret = connect(serverfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        while (true) {
            ret = recvfrom(serverfd, data, sizeof(data), 0, NULL, 0);
            ret = sendto(serverfd, &ret, sizeof(ret), 0, NULL, 0);
            //ret = write(serverfd, data, len);
            //ret = read(serverfd, &ret, sizeof(ret));  //block
        }
    } else {
        ALOGE("ipcc init failed, errno: %d (%s)", serverfd, strerror(errno));
    }
#endif
#if 1
    struct sockaddr_in servaddr, recvaddr, cliaddr;
    int32_t receive, listenfd, client, clientfd, ret;
    socklen_t clilen = sizeof(cliaddr);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    //servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    servaddr.sin_port = htons(2333);

    uint32_t* buf[1024], str[64];

    for (int32_t s = socket(AF_INET, SOCK_STREAM, 0), _m = 1; _m; _m--, s > 0 && close(s)) {
    //if ((receive = socket(AF_INET, SOCK_DGRAM, 0)) != -1) {
        if (bind(s, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) == 0) {
            listen(s, 20);
            while (true) {
                client = accept(s, (struct sockaddr *)&cliaddr, &clilen)));
                //ret = recvfrom(s, &buf, 1024, 0,(struct sockaddr *)&cliaddr, &clilen);
                //ret = sendto(s, &ret, sizeof(ret), 0, (sockaddr *)&cliaddr, sizeof(sockaddr_in));
                printf("received from %s at PORT %d\n", inet_ntop(AF_INET, 
                            &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
                while (true) {
                    read(s, buf, len);        //socket read
                    //ret = write(output, buf, n);    //file IO
                    write(s, &ret, 4);        //socket write
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
    i2c_write(0, NULL);

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
