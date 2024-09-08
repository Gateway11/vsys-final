/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "AHAL: VirtualMic"
#define LOG_NDDEBUG 0

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <map>
#include <list>
#include <condition_variable>
#include <sys/time.h>
#if 0
#include <log/log.h>

#ifdef DYNAMIC_LOG_ENABLED
#include <log_xml_parser.h>
#define LOG_MASK HAL_MOD_FILE_FM
#include <log_utils.h>
#endif
#else
#define AHAL_ERR printf
#define AHAL_DBG printf
#endif
#include "MemoryManager.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

//#define SOCKET_PATH "/dev/socket/audioserver/virtual_mic"
#define SOCKET_PATH "/tmp/unix_socket"
#define BUFFER_SIZE 3840
enum track_type_t{
    DEFAULT = 0,
    BTCALL
};

enum op_type_t {
    HANDSHAKE = 100,
    STATE_ENABLE = 1,
    STATE_DISABLE = 2,
};

std::map<track_type_t, std::list<uint8_t*>> map_tracks;
std::map<track_type_t, std::shared_ptr<MemoryManager>> map_memorys;

int32_t g_clientfd = -1;
op_type_t g_type;
std::mutex mutex;
std::condition_variable condition;

void clear_socket(int32_t sock) {
    char buf[2048];
    ssize_t bytes_read;

    while ((bytes_read = recv(sock, buf, sizeof(buf), MSG_DONTWAIT)) > 0);
    if (bytes_read < 0 && errno != EWOULDBLOCK) {
        AHAL_ERR("Error reading socket buffer: %s", strerror(errno));
    } else {
        AHAL_DBG("Socket buffer cleared successfully.");
    }
}

void virtual_mic_control(op_type_t type) {
    g_type = type;
    if (g_clientfd > 0) {
        if (write(g_clientfd, &type, sizeof(type)) < 0) {
            AHAL_ERR("Failed to write msg, error=%s", strerror(errno));
        }
    }
}

void virtual_mic_start(track_type_t type) {
    std::lock_guard<std::mutex> lg(mutex);
    //map_tracks.emplace(type, std::list<uint8_t*>());
    if (!map_tracks.size()) {
        clear_socket(g_clientfd);
        virtual_mic_control(STATE_ENABLE);
    }

    size_t total_memorysize = 1 * 1024 * 1024;  // 1MB
    size_t blockSize = BUFFER_SIZE;  // Block size of 3840 bytes

    map_tracks.try_emplace(type, std::list<uint8_t*>());
    map_memorys.try_emplace(type, std::make_shared<MemoryManager>(total_memorysize, blockSize));
    condition.notify_all();
}

void virtual_mic_stop(track_type_t type) {
    std::lock_guard<std::mutex> lg(mutex);
    map_tracks.erase(type);
    map_memorys.erase(type);

    if (!map_tracks.size()) {
        virtual_mic_control(STATE_DISABLE);
    }
}

ssize_t virtual_mic_read(track_type_t type, uint8_t* buf, ssize_t size) {
    ssize_t bytes_read = 0, time = 3;
    while (time--) {
        printf("dddddddddddddddddddddddddddd %zd, %zu\n", time, map_tracks[type].size());
        {
            std::lock_guard<std::mutex> lg(mutex);
            if (map_memorys[type] != nullptr && map_tracks[type].size()) {
                uint8_t* block = map_tracks[type].front();
                if (block != nullptr) {
                    memcpy(buf, block, size);

                    map_tracks[type].pop_front();
                    map_memorys[type]->deallocate(block);
                    bytes_read = size;
                    break;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    return bytes_read;
}

void recv_thread(int32_t clientfd) {
    uint8_t buf[BUFFER_SIZE];
    std::unique_lock<decltype(mutex)> locker(mutex, std::defer_lock);

    struct timeval tv = { .tv_usec = 300/* ms */ * 1000 };
    if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        AHAL_ERR("Failed to set socket receive timeout, error=%s", strerror(errno));
        goto exit_thread;
    }
    while (true) {
        ssize_t bytes_read = 0, received;
        while(true) {
            received = read(clientfd, buf + bytes_read, BUFFER_SIZE - bytes_read);
            if (received > 0) {
                bytes_read += received;
                //AHAL_DBG("Received %zd bytes, total received = %zd, %lu\n",
                //        received, bytes_read, map_tracks.size());
                if (bytes_read != BUFFER_SIZE) continue;
                break;
            } else if (received == 0) {
                AHAL_ERR("Client disconnected.\n");
                goto exit_thread;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                AHAL_ERR("Receive timed out, no data received for 5 seconds\n");
            } else {
                AHAL_ERR("Failed to receive data, error=%s\n", strerror(errno));
            }
            goto exit_thread;
        }
        locker.lock();
        if (map_tracks.empty()) {
            printf("dddddddddddddddddddddddddddddddddd  %d\n", __LINE__);
            condition.wait(locker, [&]{ return !map_tracks.empty(); });
        } else {
            for (auto& track: map_tracks) {
                void* block = map_memorys[track.first]->allocate();
                if (block != nullptr) {
                    memcpy(block, buf, bytes_read);
                    track.second.push_back((uint8_t *)block);
                } else {
                    printf("Porcess overflow\n");
                }
            }
        }
        locker.unlock();
    }
exit_thread:
    close(clientfd);
    AHAL_DBG("exit\n");
}

void accept_thread(int serverfd) {
    AHAL_DBG("enter\n");
    while (true) {
        int32_t clientfd = accept(serverfd, nullptr, nullptr);
        if (clientfd < 0) {
            AHAL_ERR("Failed to accept connection, error=%s", strerror(errno));
            continue;
        }

        AHAL_DBG("Client connected!\n");

        op_type_t type;
        int32_t bytes_read = read(clientfd, &type, sizeof(type));
        if (bytes_read <= 0) {
            AHAL_ERR("Failed to receive msg, error=%s", strerror(errno));
            close(clientfd);
            continue;
        }

        if (type != HANDSHAKE) {
            AHAL_ERR("Invalid handshake message received. Expected HANDSHAKE.");
            close(clientfd);
            continue;
        }
        std::thread client_recv_thread(recv_thread, clientfd);
        client_recv_thread.detach();

        g_clientfd = clientfd;
        virtual_mic_control(g_type);
    }
    AHAL_DBG("exit\n");
}

int32_t virtual_mic_init() {
    AHAL_DBG("enter\n");

    int32_t serverfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverfd < 0) {
        AHAL_ERR("Failed to create socket, error=%s", strerror(errno));
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH); // Remove previous socket file
    if (bind(serverfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        AHAL_ERR("Failed to bind socket, error=%s", strerror(errno));
        close(serverfd);
        return -1;
    }

    if (listen(serverfd, 5) < 0) {
        AHAL_ERR("Failed to listen on socket, error=%s", strerror(errno));
        close(serverfd);
        return -1;
    }

    AHAL_DBG("Server is waiting for connections...\n");
    std::thread accept_thread_handle(accept_thread, serverfd);
    accept_thread_handle.detach();

    AHAL_DBG("exit\n");
    return 0;
}

#if 1
#include <fstream>
std::ofstream output("./44100.2.16bit.wav", std::ios::out | std::ios::binary);
int32_t main() {
    uint8_t buf[BUFFER_SIZE];

    virtual_mic_init();
    virtual_mic_start(DEFAULT);
    while (true) {
        ssize_t ret = virtual_mic_read(DEFAULT, buf, BUFFER_SIZE);
        if (ret)
            output.write((const char *)buf, BUFFER_SIZE);
    }
    virtual_mic_stop(DEFAULT);
}
#endif

//#ifdef __cplusplus
//}
//#endif

