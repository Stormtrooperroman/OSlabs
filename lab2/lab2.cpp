#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <string>

const int MAX_CLIENTS = 2;
const int BUFFER_SIZE = 1024;

class Client {
public:
    int connfd;
    sockaddr_in addr;
};


void printClientAddress(const sockaddr_in& addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    std::cout << "[" << ip << ":" << htons(addr.sin_port) << "]";
}

volatile sig_atomic_t wasSignal = 0;

void signalHandler(int /*signum*/) {
    wasSignal = 1;
}

void setupSignalHandler(sigset_t* origMask) {
    struct sigaction sa;
    sigaction(SIGHUP, nullptr, &sa);
    sa.sa_handler = signalHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, nullptr);

    sigset_t blockedMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, origMask);
}

int createServer(int port) {
    int sockfd;
    sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        exit(-1);
    }
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        std::cerr << "Socket bind failed" << std::endl;
        exit(-1);
    }

    if (listen(sockfd, MAX_CLIENTS) != 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(-1);
    }

    return sockfd;
}

void runServer(int sockfd) {
    Client clients[MAX_CLIENTS] = {};
    int active_clients = 0;
    std::string buffer;

    sigset_t origSigMask;
    setupSignalHandler(&origSigMask);

    while (true) {
        if (wasSignal) {
            wasSignal = 0;
            std::cout << "Clients: ";
            for (int i = 0; i < active_clients; i++) {
                printClientAddress(clients[i].addr);
                std::cout << " ";
            }
            std::cout << std::endl;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        int maxFd = sockfd;

        for (int i = 0; i < active_clients; i++) {
            FD_SET(clients[i].connfd, &fds);
            if (clients[i].connfd > maxFd) {
                maxFd = clients[i].connfd;
            }
        }

        if (pselect(maxFd + 1, &fds, nullptr, nullptr, nullptr, &origSigMask) < 0
            && errno != EINTR) {
            std::cerr << "pselect failed" << std::endl;
            return;
        }

        if (FD_ISSET(sockfd, &fds) && active_clients < MAX_CLIENTS) {
            Client* client = &clients[active_clients];
            socklen_t len = sizeof(client->addr);
            int connfd = accept(sockfd, (struct sockaddr*)&client->addr, &len);
            if (connfd >= 0) {
                client->connfd = connfd;
                printClientAddress(clients->addr);
                std::cout << " Connected!" << std::endl;
                active_clients++;
            } else {
                std::cout << "Accept error: " << strerror(errno) << std::endl;
            }
        }

        for (int i = 0; i < active_clients; i++) {
            Client* client = &clients[i];
            if (FD_ISSET(client->connfd, &fds)) {
                buffer.clear();
                int read_len = read(client->connfd, &buffer[0], BUFFER_SIZE - 1);
                if (read_len > 0) {
                    buffer[read_len - 1] = 0;
                    printClientAddress(clients->addr);
                    std::cout << " Message: " << buffer.c_str() << std::endl;
                } else {
                    close(client->connfd);
                    printClientAddress(clients->addr);
                    std::cout << " Connection closed" << std::endl;
                    clients[i] = clients[active_clients - 1];
                    active_clients--;
                    i--;
                }
            }
        }
    }
}


int main() {
    int sockfd = createServer(5005);
    std::cout << "Listening..." << std::endl;
    runServer(sockfd);
    return 0;
}
