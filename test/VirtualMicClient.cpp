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
#include <sys/time.h>
#include <thread>
#include <map>
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

#if 0
#ifdef __cplusplus
extern "C" {
#endif
#endif

//#define SOCKET_PATH "/dev/socket/audioserver/virtual_mic"
#define SOCKET_PATH "/tmp/unix_socket"

enum track_type_t{
    DEFAULT = 0,
    BTCALL
};

struct Track;
std::map<track_type_t, std::shared_ptr<Track>> tracks;

struct Track {
    int32_t sock;
    track_type_t type__;
    bool is_connected;

    Track(track_type_t type): type__(type), is_connected(false) {
        sock = socket(PF_UNIX, SOCK_STREAM, 0); 
        if(sock < 0) {
            AHAL_ERR("Failed to open socket, error=%s", strerror(errno));
            return;
        }   

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1); 

 //       std::thread thread([&]{
            while(1) {
                int32_t ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
                printf("%d, ret=%d\n", __LINE__, ret);
                if(ret == 0)
                    break;

                printf("%d\n", __LINE__);
                AHAL_ERR("Connect to server '%s' failed, error=%s\n", SOCKET_PATH, strerror(errno));
                usleep(10*1000); //10ms
                printf("%d\n", __LINE__);
            }   

            AHAL_DBG("Connected to server '%s' success, socket id=%d", SOCKET_PATH, sock);

            struct timeval tv = { .tv_usec = 300/* ms */ * 1000 };
            if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                AHAL_ERR("Failed to set socket receive timeout, error=%s", strerror(errno));
                close(sock);
            }
            is_connected = true;
 //       });
 //       thread.detach();
    }

    void print_socket_buffer_size() {
        int32_t rcvbuf_size;
        socklen_t optlen = sizeof(int32_t);
    
        if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, &optlen) == -1) {
            AHAL_ERR("getsockopt (SO_RCVBUF) failed, error=%s", strerror(errno));
            return;
        }
        AHAL_DBG("Default receive buffer size: %d bytes", rcvbuf_size);
    }

    ssize_t read_data(uint8_t* buf, ssize_t size) {
        ssize_t bytes_read = 0, received;
        if (is_connected) {
            while(true) {
                received = read(sock, buf + bytes_read, size - bytes_read);
                if (received > 0) {
                    bytes_read += received;
                    AHAL_DBG("Received %zd bytes, total received = %zd\n", received, bytes_read);
                    if (bytes_read != size) continue;
                } else if (received == 0) {
                    AHAL_ERR("Client disconnected.");
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    AHAL_ERR("Receive timed out, no data received for 5 seconds");
                } else {
                    AHAL_ERR("Failed to receive data, error=%s", strerror(errno));
                }
                break;
            }
        }
        return bytes_read;
    }
};

void virtual_mic_start(track_type_t type) {
    tracks.try_emplace(type, new Track(type));
}

void virtual_mic_stop(track_type_t type) {
    tracks.erase(type);
}

ssize_t virtual_mic_read(track_type_t type, uint8_t* buf, ssize_t size) {
    auto it = tracks.find(type);
    if (it == tracks.end()) {
        virtual_mic_start(type);
    }
    return tracks[type]->read_data(buf, size);
}

#if 1
#include <fstream>
int32_t main() {
    uint8_t buf[3840];
    //std::ofstream output("./16000.2.16bit.pcm", std::ios::out | std::ios::binary);
    std::ofstream output("./16000.2.16bit.wav", std::ios::out | std::ios::binary);

    virtual_mic_start(DEFAULT);
    while (true) {
        ssize_t bytes_read = virtual_mic_read(DEFAULT, buf, sizeof(buf));;
        if (bytes_read <= 0) break;
        output.write((const char *)buf, bytes_read);
    }
    virtual_mic_start(DEFAULT);
}
#else

#ifdef __cplusplus
}
#endif

#endif
