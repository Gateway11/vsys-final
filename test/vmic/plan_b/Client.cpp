#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <thread>

#define SOCKET_PATH "/tmp/unix_socket"
#define BUFFER_SIZE 3840

void send_thread(int32_t sock) {
    uint8_t buf[BUFFER_SIZE];
    //std::ifstream input("/sdcard/Music/16000.4.16bit.pcm", std::ios::in | std::ios::binary);
    std::ifstream input("/Users/daixiang/Music/test.wav", std::ios::in | std::ios::binary);

    while (true) {
        if (input.good()) {
            input.read((char *)buf, sizeof(buf));

            int32_t bytes_sent = send(sock, buf, sizeof(buf), MSG_NOSIGNAL);
            if (bytes_sent == -1) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    printf("Server has disconnected.\n");
                } else {
                    printf("Failed to send data.\n");
                }   
                break;
            }   
            //std::this_thread::sleep_for(std::chrono::milliseconds(15));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            input.clear();
            input.seekg(0, std::ios::beg);
        }   
    }   
}

int main() {
    int32_t sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Failed to create socket\n");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    while(1) {
        int32_t ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        if(ret == 0)
            break;

        printf("Connect to server '%s' failed, %s\n", SOCKET_PATH, strerror(errno));
        usleep(10*1000); //10ms
    }

    printf("Connected to server '%s' success, socket id=%d\n", SOCKET_PATH, sock);
    send_thread(sock);
#if 0
    Message msg;

    msg.cmd = HANDSHAKE;  // No need to set data for HANDSHAKE
    if (write(sock, &msg, sizeof(msg)) < 0) {
        printf("Failed to send handshake message\n");
        close(sock);
        return -1;
    }

    int bytes_read = read(sock, &msg, sizeof(msg));
    if (bytes_read > 0) {
        // Check if the command is SET_STATE
        if (msg.cmd == SET_STATE) {
            // Check the state in the data[0] field
            if (msg.data[0] == '1') {
                printf("Received SET_STATE command: ENABLE\n");
                // Here you can add logic to enable the feature
            } else if (msg.data[0] == '0') {
                printf("Received SET_STATE command: DISABLE\n");
                // Here you can add logic to disable the feature
            } else {
                printf("Received SET_STATE command with invalid data\n");
            }
        } else {
            printf("Received unknown command\n");
        }
    } else {
        printf("Failed to receive SET_STATE command\n");
    }

    // Close socket
    close(sock);
#endif
    return 0;
}
