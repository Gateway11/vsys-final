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
#include <sys/time.h>
#include <log/log.h>

#ifdef DYNAMIC_LOG_ENABLED
#include <log_xml_parser.h>
#define LOG_MASK HAL_MOD_FILE_FM
#include <log_utils.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET_PATH "/dev/socket/audioserver/virtual_mic"

static int32_t g_clientfd;
struct Message {
    int cmd;         // Command type (e.g., HANDSHAKE, SET_STATE)
                     // Command type (e.g., HANDSHAKE, ENABLE, DISABLE)
    char data[32];
};

enum Command {
    HANDSHAKE = 1,
    // Uses the data field to specify the state: data[0] = 0 for DISABLE, data[0] = 1 for ENABLE
    SET_STATE = 2
};

int32_t virtual_mic_read(uint8_t* buf, ssize_t size) {
    int32_t bytes_read = 0;
    if (clientfd > 0) {
        bytes_read = read(clientfd, buf, size);
        if (bytes_read > 0) {
            AHAL_DBG("Received data, num_read=%d", num_read);
        } else if (bytes_read == 0) {
            AHAL_ERR("Client disconnected.");
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            AHAL_ERR("Receive timed out, no data received for 5 seconds");
        } else {
            AHAL_ERR("Failed to receive data, error=%s", strerror(errno));
        }
    }
    return bytes_read < 0 ? 0 : bytes_read;
}

void clear_socket(int32_t sock) {
    char buffer[2048];
    int32_t bytes_read;

    while ((bytes_read = recv(sock, buffer, sizeof(buffer), MSG_DONTWAIT)) > 0);
    if (bytes_read < 0 && errno != EWOULDBLOCK) {
        AHAL_ERR("Error reading socket buffer: %s\n", strerror(errno));
    } else {
        AHAL_DBG("Socket buffer cleared successfully.");
    }
}

void virtual_mic_control(Command type, void *data, ssize_t size) {
    Message msg;

    if (type == SET_STATE && *(bool *)data) {
        clear_socket(clientfd);
    }

    msg.cmd = type;
    memcpy(msg.data, data, size);
    if (write(clientfd, &msg, sizeof(msg)) < 0) {
        AHAL_ERR("Failed to write msg, error=%s", strerror(errno));
    }   
}

void accept_thread(int serverfd) {
    Message msg;

    AHAL_DBG("enter");
    while (true) {
        int32_t clientfd = accept(serverfd, nullptr, nullptr);
        if (clientfd < 0) {
            AHAL_ERR("Failed to accept connection, error=%s", strerror(errno));
            continue;
        }

        AHAL_DBG("Client connected!");

        struct timeval tv = { .tv_usec = 300/* ms */ * 1000 };
        if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            AHAL_ERR("Failed to set socket receive timeout, error=%s", strerror(errno));
            close(clientfd);
            continue;
        }

        int32_t bytes_read = read(clientfd, &msg, sizeof(msg));
        if (bytes_read <= 0) {
            AHAL_ERR("Failed to receive msg, error=%s", strerror(errno));
            close(clientfd);
            continue;
        }

        if (msg.cmd != HANDSHAKE) {
            AHAL_ERR("Invalid handshake message received. Expected HANDSHAKE.");
            close(clientfd);
            continue;
        }
        g_clientfd = clientfd;
    }
    AHAL_DBG("exit\n");
}

int32_t ipc_server_socket_init() {
    AHAL_DBG("enter");

    g_clientfd = -1;
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

    AHAL_DBG("Server is waiting for connections...");
    std::thread accept_thread_handle(accept_thread, serverfd);
    accept_thread_handle.detach();

    AHAL_DBG("exit");
    return 0;
}

#ifdef __cplusplus
}
#endif
