#include <iostream>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>

#define MAX_EVENTS 10
#define PORT 8080

// Function to set socket to non-blocking mode
void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Create a TCP socket
    int listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set socket to non-blocking mode
    set_nonblocking(listen_sockfd);

    // Set up server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(listen_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(listen_sockfd, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Start the epoll event loop to accept connections
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev, events[MAX_EVENTS];
    
    // Register the listening socket with epoll
    ev.events = EPOLLIN;
    ev.data.fd = listen_sockfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sockfd, &ev) == -1) {
        perror("epoll_ctl: EPOLL_CTL_ADD");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is ready to accept connections...\n";

    // Event loop
    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_sockfd) {
                // Accept new connection
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_sockfd = accept(listen_sockfd, (struct sockaddr*)&client_addr, &client_len);
                if (client_sockfd == -1) {
                    perror("accept");
                    continue;
                }

                std::cout << "New connection accepted\n";
                set_nonblocking(client_sockfd);

                // Register the new client socket with epoll
                ev.events = EPOLLIN | EPOLLET;  // Edge-triggered mode
                ev.data.fd = client_sockfd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sockfd, &ev) == -1) {
                    perror("epoll_ctl: EPOLL_CTL_ADD");
                    exit(EXIT_FAILURE);
                }
            } else if (events[i].events & EPOLLIN) {
                int sockfd = events[i].data.fd;
                // Perform the read operation in the current thread
                char buffer[1024];
                ssize_t n = read(sockfd, buffer, sizeof(buffer));
                if (n > 0) {
                    // Process the data that has been read
                } else if (n == 0) {
                    std::cout << "Client disconnected\n";
                } else {
                    perror("read");
                }
            }
        }
    }

    // Close the listening socket (this will never be reached in this example)
    close(listen_sockfd);

    return 0;
}
