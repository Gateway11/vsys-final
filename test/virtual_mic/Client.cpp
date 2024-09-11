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

struct Message {
    op_type_t type;         // Command type (e.g., HANDSHAKE, STATE_ENABLE, STATE_DISABLE)
};

op_type_t g_type = HANDSHAKE;
std::mutex mutex;
std::condition_variable condition;
std::ifstream input;

void print_socket_buffer_size(int32_t sock) {
    int32_t sendbuf_size;
    socklen_t optlen = sizeof(int32_t);

    if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf_size, &optlen) == -1) {
        printf("getsockopt (SO_SNDBUF) failed, error=%s\n", strerror(errno));
        return;
    }
    printf("Default send buffer size: %d KB\n", sendbuf_size / 1024);
}

void send_thread(int32_t sock) {
    uint8_t buf[BUFFER_SIZE];
    std::unique_lock<decltype(mutex)> locker(mutex, std::defer_lock);

    //int32_t sendbuf_size = 1 * 1024 * 1024;  // 1MB
    //setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuf_size, sizeof(sendbuf_size));
    //print_socket_buffer_size(sock);

    while (true) {
        locker.lock();
        if (g_type != STATE_ENABLE) {
            condition.wait(locker, [&]{ return g_type != STATE_ENABLE; });
            input.clear();
            input.seekg(0, std::ios::beg);
        }
        locker.unlock();
        if (input.good()) {
            input.read((char *)buf, sizeof(buf));
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            int32_t bytes_sent = send(sock, buf, sizeof(buf), MSG_NOSIGNAL);
            if (bytes_sent == -1) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    printf("%s, Server disconnected.\n", __func__);
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
    close(sock);
}

void recv_thread(int32_t sock) {
    Message msg;

    while (true) {
        ssize_t num_read = read(sock, &msg, sizeof(msg));
        if (num_read > 0) {
            printf("num_read =%zd, Received message: %d\n", num_read , msg.type);
            g_type = msg.type;
            condition.notify_one();
        } else if (num_read == 0) {
            printf("%s, Server disconnected.\n", __func__);
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

    Message msg;
    msg.type = g_type;
    if (write(sock, &msg, sizeof(msg)) < 0) {
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
