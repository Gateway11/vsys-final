#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fstream>
#include <list>
#include <thread>
#include <condition_variable>

//#define SOCKET_PATH "/dev/socket/audioserver/virtual_mic"
#define SOCKET_PATH "/tmp/unix_socket"

std::list<int32_t> sockets;
std::mutex mutex;
std::condition_variable condition;

void send_thread() {
    uint8_t buf[3840];
    //std::ifstream input("/sdcard/Music/16000.4.16bit.pcm", std::ios::in | std::ios::binary);
    std::ifstream input("/Users/daixiang/Music/test.wav", std::ios::in | std::ios::binary);
    std::unique_lock<decltype(mutex)> locker(mutex, std::defer_lock);

    while (true) {
        int32_t sock_tmp = -1;

        locker.lock();
        if (sockets.empty()) {
            condition.wait(locker, [&]{ return !sockets.empty(); });
        }
        if (input.good()) {
            input.read((char *)buf, sizeof(buf));

            for (auto sock: sockets) {
                int32_t bytes_sent = send(sock, buf, sizeof(buf), MSG_NOSIGNAL);
                if (bytes_sent == -1) {
                    if (errno == EPIPE || errno == ECONNRESET) {
                        printf("Client has disconnected.\n");
                        sock_tmp = sock;
                    } else {
                        printf("Failed to send data.\n");
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            input.clear();
            input.seekg(0, std::ios::beg);
        }
        if (sock_tmp > 0) sockets.remove(sock_tmp);
        locker.unlock();
    }
}

int32_t main() {
    int32_t serverfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverfd < 0) {
        printf("Failed to create socket, error=%s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH); // Remove previous socket file
    if (bind(serverfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Failed to bind socket, error=%s\n", strerror(errno));
        close(serverfd);
        return -1;
    }

    if (listen(serverfd, 100) < 0) {
        printf("Failed to listen on socket, error=%s\n", strerror(errno));
        close(serverfd);
        return -1;
    }

    std::thread send_thread_handle(send_thread);
    send_thread_handle.detach();

    printf("Server is waiting for connections...\n");
    while (true) {
        int32_t clientfd = accept(serverfd, nullptr, nullptr);
        if (clientfd < 0) {
            printf("Failed to accept connection, error=%s\n", strerror(errno));
            continue;
        }

        printf("Client connected!, sock=%d\n", clientfd);
        std::lock_guard<std::mutex> lg(mutex);
        sockets.push_back(clientfd);
        condition.notify_one();
    }
    return 0;
}
