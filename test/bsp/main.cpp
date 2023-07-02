//
//  main.cpp
//  audio_tools
//
//  Created by 代祥 on 2023/3/31.
//

#include <fstream>
#include <thread>
#include <vector>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
//#include <linux/types.h>
//#include <linux/in.h>
//#include <netinet/in.h>
//#include <linux/inet.h> //htonl/inet_pton
//#include <arpa/inet.h> //htonl/inet_pton

#include <fcntl.h>
#include <sys/stat.h>

#define IPC_DATA_MAX_SIZE 1024

#define DP_CR5_SAF 0
#define AUDIO_MONITOR_EPT 125 //8023

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

int output = open("/data/test.txt", O_WRONLY|O_CREAT|O_APPEND, S_IWUSR|S_IRGRP|S_IROTH);
int main(int argc, const char * argv[]) {
    //hal client
    //https://www.demo2s.com/c/c-sock-socket-af-rpmsg-sock-seqpacket-0.html
    //SemiDrive_X9_音频应用指南_Rev1.1.pdf -43
#if 1
    int32_t remote = 0, ret;
    struct timeval tv = { .tv_usec = 300/* ms */ * 1000 };

    uint32_t* data[1024], len;

    struct sockaddr_rpmsg servaddr, dst_addr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.family = AF_RPMSG;
    servaddr.vproc_id = DP_CR5_SAF;
    servaddr.addr = AUDIO_MONITOR_EPT;

    ALOGD("--------------- %d", __LINE__);
    if ((remote = socket(PF_RPMSG, SOCK_SEQPACKET, 0)) != -1) {
        if (setsockopt(remote, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
            ALOGE("Set rcv timeo failed");
        if (setsockopt(remote, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
            ALOGE("Set snd timeo failed");
        ret = connect(remote, (struct sockaddr *)&servaddr, sizeof(servaddr));
        while (true) {
            ret = write(remote, data, len);
            ret = read(remote, &ret, sizeof(ret));  //block
        }
    } else {
        ALOGE("ipcc init failed, errno: %d (%s)", remote, strerror(errno));
    }
#else
    int32_t listenfd, clientfd, ret;
    uint32_t* data[1024], len;

    struct sockaddr_rpmsg recvaddr, cliaddr;
    bzero(&recvaddr, sizeof(recvaddr));
    recvaddr.family = AF_RPMSG;
    recvaddr.vproc_id = DP_CR5_SAF;
    recvaddr.addr = AUDIO_MONITOR_EPT;

    socklen_t clilen = sizeof(cliaddr);

    if ((listenfd = socket(PF_RPMSG, SOCK_SEQPACKET, 0)) != -1) {
        if (bind(listenfd, (struct sockaddr *)&recvaddr, sizeof(recvaddr)) != -1);
        if (listen(listenfd, 5) == 0);
        if ((clientfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) != -1) {
            printf("received from %s at PORT %d\n", inet_ntop(AF_INET,
                        &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
        }
        while (true) {
            //ret = recvfrom(listenfd, data, sizeof(data), 0, &cliaddr, &clilen);
            ret = read(clientfd, data, sizeof(data));  //block
            ret = write(clientfd, data, len);
        }
    } else {
        ALOGE("ipcc init failed, errno: %d (%s)", listenfd, strerror(errno));
    }
#endif


#if 0
    // safety server tcp/udp
    int32_t receive, client, ret;
    struct sockaddr_in recvaddr, cliaddr;
    memset(&recvaddr, 0, sizeof(recvaddr));
    socklen_t cliaddr_len = sizeof(cliaddr);

    char ip_addr_str[32], str[32];
    snprintf(ip_addr_str, sizeof(ip_addr_str), "%d.%d.%d.%d", ULINK_NET_NS0,
             ULINK_NET_NS1, ULINK_NET_NS2, ULINK_IP_OFFSET/* + DP_CR5_SAF*/);

    //ip4_addr_t ip;
    //inet_aton(ip_addr_str, &ip);
    //inet_addr_from_ip4addr(&recvaddr.sin_addr, &ip);

    recvaddr.sin_family = AF_INET; //PF_INET
    recvaddr.sin_addr.s_addr = inet_addr(ip_addr_str);
    recvaddr.sin_port = PP_HTONS/*htons*/(SSA_LWIP_SERVER_PORT /* 8023 */);
    //recvaddr.sin_len = sizeof(recvaddr);

    if ((receive = lwip_socket(AF_INET, SOCK_STREAM, 0)) >= 0) {
    //if ((receive = lwip_socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) >= 0) {
        if (lwip_bind(receive, (struct sockaddr *)&recvaddr, sizeof(recvaddr)) == 0) {
            //struct timeval tv = { .tv_sec = 300/* ms */ * 1000, .tv_usec = 0 };
            //setsockopt(receive, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            lwip_listen(socket, 20);
            while (true) {
                client = lwip_accept(receive, (struct sockaddr *)&cliaddr, &cliaddr_len);
                //ret = lwip_recvfrom(receive, disp->recv_data, RECV_DATA, 0, &cliaddr, &cliaddr_len);
                //need conf ?
                //cliaddr.sin_family = PF_INET;
                //cliaddr.sin_port = htons(8888);
                //cliaddr.sin_addr.s_addr = inet_addr("172.20.2.41");
                //lwip_sendto(receive, (char *)buf, len, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
                printf("received from %s at PORT %d\n", inet_ntop(AF_INET,
                            &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
                // TODO: check client IP is SSB ?
                while (true) {
                    ret = lwip_recv(client, &recv_data, sizeof(recv_data), 0);
                    lwip_send(client, &ret, sizeof(ret), 0);
                }
            }
            lwip_close(client);
            lwip_close(receive);
        }
    }
#endif
#if 0
    //linux server tcp/udp
    struct sockaddr_in servaddr, cliaddr;
    int32_t receive, client, ret;
    socklen_t cliaddr_len = sizeof(cliaddr);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(2333);

    uint32_t* buf[1024], str[64];

    if ((receive = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
    //if ((recv_socket = socket(AF_INET, SOCK_DGRAM, 0)) != -1) {
        if (bind(receive, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) == 0) {
            listen(recv_socket, 20);
            while (true) {
                client = accept(receive, (struct sockaddr *)&cliaddr, &cliaddr_len)));
                //ret = recvfrom(receive, &buf, 1024, 0,(struct sockaddr *)&cliaddr, &cliaddr_len);
                //ret = sendto(receive, &ret, sizeof(ret), 0, (sockaddr *)&cliaddr, sizeof(sockaddr_in));
                printf("received from %s at PORT %d\n", inet_ntop(AF_INET, 
                            &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
                while (true) {
                    read(receive, buf, len);        //socket read
                    //ret = write(output, buf, n);    //file IO
                    write(receive, &ret, 4);        //socket write
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
