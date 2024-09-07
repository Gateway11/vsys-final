#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <sys/time.h>

#define SOCKET_PATH "/tmp/unix_socket_example"

// Message structure for data transfer
struct Message {
    int cmd;         // Command type (e.g., HANDSHAKE, SET_STATE)
    char data[32];
};

// Updated command types
enum Command {
    HANDSHAKE = 1,
    // Uses the data field to specify the state: data[0] = 0 for DISABLE, data[0] = 1 for ENABLE
    SET_STATE = 2
};

// Function to handle receiving data after handshake
void recv_thread(int clientfd) {
    Message msg;

    while (true) {
        int bytes_read = read(clientfd, &msg, sizeof(msg));
        if (bytes_read > 0) {
            switch (msg.cmd) {
                case SET_STATE:
                    if (msg.data[0] == '1') {
                        printf("Received SET_STATE command: ENABLE\n");
                    } else if (msg.data[0] == '0') {
                        printf("Received SET_STATE command: DISABLE\n");
                    } else {
                        printf("Received SET_STATE command with invalid data\n");
                    }
                    break;
                default:
                    printf("Received unknown command\n");
            }
        } else if (bytes_read == 0) {
            printf("Client disconnected.\n");
            break;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("Receive timed out, no data received for 5 seconds\n");
        } else {
            printf("Failed to receive data\n");
            break;
        }
    }
}

// Function to handle handshake and send SET_STATE command to the client
void handle_client(int clientfd) {
    Message msg;

    // Step 1: Server receives the handshake message from the client
    int bytes_read = read(clientfd, &msg, sizeof(msg));
    if (bytes_read <= 0) {
        printf("Failed to receive handshake message or client disconnected during handshake.\n");
        close(clientfd);
        return;
    }

    // Step 2: Check if cmd is HANDSHAKE
    if (msg.cmd != HANDSHAKE) {
        printf("Invalid handshake message received. Expected HANDSHAKE.\n");
        close(clientfd);
        return;
    }

    printf("Handshake message received successfully from client.\n");

    // Step 3: Send a handshake reply to the client
    msg.cmd = HANDSHAKE;
    if (write(clientfd, &msg, sizeof(msg)) < 0) {
        printf("Failed to send handshake reply.\n");
        close(clientfd);
        return;
    }

    printf("Handshake reply sent to client.\n");

    // Step 4: After handshake, send a SET_STATE command to the client
    msg.cmd = SET_STATE;
    msg.data[0] = '1';  // Send '1' to enable a feature
    if (write(clientfd, &msg, sizeof(msg)) < 0) {
        printf("Failed to send SET_STATE command: ENABLE\n");
        close(clientfd);
        return;
    }

    printf("Sent SET_STATE command to client: ENABLE.\n");

    // Optional: You can send another SET_STATE command later (e.g., to disable)
    // msg.data[0] = '0';  // Send '0' to disable a feature
    // if (write(clientfd, &msg, sizeof(msg)) < 0) {
    //     printf("Failed to send SET_STATE command: DISABLE\n");
    // }

    // Step 5: Start a recv_thread to handle further communication (if needed)
    std::thread client_recv_thread(recv_thread, clientfd);
    client_recv_thread.detach(); // Detach to allow independent handling
}

// Function to handle receiving data after handshake
void recv_thread_noreply(int clientfd) {
    Message msg;

    while (true) {
        int bytes_read = read(clientfd, &msg, sizeof(msg));
        if (bytes_read > 0) {
            printf("num_read =%zu, Received message: %s\n", num_read , msg.data);
        } else if (bytes_read == 0) {
            printf("Client disconnected.\n");
            break;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("Receive timed out, no data received for 5 seconds\n");
        } else {
            printf("Failed to receive data\n");
            break;
        }
    }
}

// Function to handle handshake and send SET_STATE command to the client
void handle_client_no_reply(int clientfd) {
    Message msg;

    // Step 1: Server receives the handshake message from the client
    int bytes_read = read(clientfd, &msg, sizeof(msg));
    if (bytes_read <= 0) {
        printf("Failed to receive handshake message or client disconnected during handshake.\n");
        close(clientfd);
        return;
    }

    // Step 2: Check if cmd is HANDSHAKE
    if (msg.cmd != HANDSHAKE) {
        printf("Invalid handshake message received. Expected HANDSHAKE.\n");
        close(clientfd);
        return;
    }

    printf("Handshake message received successfully from client.\n");

    // Step 3: Directly send a SET_STATE command to the client after receiving HANDSHAKE
    msg.cmd = SET_STATE;
    msg.data[0] = '1';  // Send '1' to enable a feature
    if (write(clientfd, &msg, sizeof(msg)) < 0) {
        printf("Failed to send SET_STATE command: ENABLE\n");
        close(clientfd);
        return;
    }

    printf("Sent SET_STATE command to client: ENABLE.\n");

    // Optional: Send another SET_STATE command later (e.g., to disable)
    // msg.data[0] = '0';  // Send '0' to disable a feature
    // if (write(clientfd, &msg, sizeof(msg)) < 0) {
    //     printf("Failed to send SET_STATE command: DISABLE\n");
    // }

    // Step 4: Start a recv_thread to handle further communication (if needed)
    std::thread client_recv_thread(recv_thread, clientfd);
    client_recv_thread.detach(); // Detach to allow independent handling
}

// Thread function to accept new clients and create a handle_client for each client
void accept_thread(int serverfd) {
    while (true) {
        int clientfd = accept(serverfd, nullptr, nullptr);
        if (clientfd < 0) {
            printf("Failed to accept connection\n");
            continue;
        }

        printf("Client connected!\n");

        struct timeval timeout;
        timeout.tv_sec = 5;  // 5 seconds timeout
        timeout.tv_usec = 0;

        if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            printf("Failed to set socket receive timeout\n");
            close(clientfd);
            return;
        }
        // Handle the client (handshake + receiving data)
        std::thread client_handle_thread(handle_client, clientfd);
        client_handle_thread.detach(); // Detach the thread to allow it to run independently
    }
}

int main() {
    int serverfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverfd < 0) {
        printf("Failed to create socket\n");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH); // Remove previous socket file
    if (bind(serverfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Failed to bind socket\n");
        close(serverfd);
        return -1;
    }

    if (listen(serverfd, 5) < 0) {
        printf("Failed to listen on socket\n");
        close(serverfd);
        return -1;
    }

    printf("Server is waiting for connections...\n");

    // Start a new thread to accept incoming client connections
    std::thread accept_thread_handle(accept_thread, serverfd);
    accept_thread_handle.join(); // Keep the main thread alive
    return 0;
}


















int main() {
    int clientfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (clientfd < 0) {
        printf("Failed to create socket\n");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(clientfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Failed to connect to server\n");
        close(clientfd);
        return -1;
    }

    Message msg;

    // Step 1: Send handshake message to server
    msg.cmd = HANDSHAKE;  // No need to set data for HANDSHAKE
    if (write(clientfd, &msg, sizeof(msg)) < 0) {
        printf("Failed to send handshake message\n");
        close(clientfd);
        return -1;
    }

    printf("Handshake sent, waiting for SET_STATE command...\n");

    // Step 2: Receive SET_STATE command from server
    int bytes_read = read(clientfd, &msg, sizeof(msg));
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

    close(clientfd);

    return 0;
}
