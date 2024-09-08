#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <thread>
#include <condition_variable>

#define SOCKET_PATH "/tmp/unix_socket"
#define BUFFER_SIZE 3840

enum op_type_t {
    HANDSHAKE = 100,
    STATE_ENABLE = 1,
    STATE_DISABLE = 2,
};

op_type_t g_type = HANDSHAKE;
std::mutex mutex;
std::condition_variable condition;
std::ifstream input;

void send_thread(int32_t sock) {
    uint8_t buf[BUFFER_SIZE];
    std::unique_lock<decltype(mutex)> locker(mutex, std::defer_lock);

    while (true) {
        locker.lock();
        if (g_type != STATE_ENABLE) {
            condition.wait(locker, [&]{ return g_type != STATE_ENABLE; });
        }
        locker.unlock();
        if (input.good()) {
            input.read((char *)buf, sizeof(buf));

            int32_t bytes_sent = send(sock, buf, sizeof(buf), MSG_NOSIGNAL);
            if (bytes_sent == -1) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    printf("Server disconnected.\n");
                } else {
                    printf("Failed to send data.\n");
                }   
                break;
            }   
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            input.clear();
            input.seekg(0, std::ios::beg);
        }   
    }   
}

void recv_thread(int32_t clientfd) {
    while (true) {
        ssize_t num_read = read(clientfd, &g_type, sizeof(g_type));
        if (num_read > 0) {
            printf("num_read =%zd, Received message: %d\n", num_read , g_type);
            condition.notify_one();
        } else if (num_read == 0) {
            printf("Server disconnected.\n");
            break;
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./client /Users/daixiang/Music/test.wav \n");
        return 0;
    }

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

    if (write(sock, &g_type, sizeof(g_type)) < 0) {
        printf("Failed to send handshake message\n");
        close(sock);
        return -1;
    }
    input.open(argv[1], std::ios::in | std::ios::binary);

    std::thread client_recv_thread(recv_thread, sock);
    client_recv_thread.detach();
    send_thread(sock);
    return 0;
}
