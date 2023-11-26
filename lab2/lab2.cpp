#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>


using namespace std;


struct client_t {
    int connfd;
    sockaddr_in addr;
};

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
        cout << "Socket creation failed" << endl;
        exit(-1);
    }
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        cout << "Socket bind failed" << endl;
        exit(-1);
    }

    if (listen(sockfd, 5) != 0) {
        cout << "Listen failed" << endl;
        exit(-1);
    }

    return sockfd;
}

int main() {
    int sockfd = createServer(5005);
    cout << "Listening..." << endl;
    client_t clients[2];
    int active_clients = 0;
    char buffer[1024] = {0};

    sigset_t origSigMask;
    setupSignalHandler(&origSigMask);

    while (true) {
        if (wasSignal) {
            wasSignal = 0;
            cout << "Clients: ";
            for (int i = 0; i < active_clients; i++) {
                printf("[%s:%d]", inet_ntoa(clients[i].addr.sin_addr),
                       htons(clients[i].addr.sin_port));
                cout << " ";
            }
            cout << endl;
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
            cout << "pselect failed" << endl;
            return -1;
        }

        if (FD_ISSET(sockfd, &fds) && active_clients < 3) {
            client_t* client = &clients[active_clients];
            socklen_t len = sizeof(client->addr);
            int connfd = accept(sockfd, (struct sockaddr*)&client->addr, &len);
            if (connfd >= 0) {
                client->connfd = connfd;
                printf("[%s:%d]", inet_ntoa(client->addr.sin_addr), htons(client->addr.sin_port));
                cout << " Connected!" << endl;
                active_clients++;
            } else {
                printf("Accept error: %s\n", strerror(errno));
            }
        }

        for (int i = 0; i < active_clients; i++) {
            client_t* client = &clients[i];
            if (FD_ISSET(client->connfd, &fds)) {
                int read_len = read(client->connfd, &buffer, 1023);
                if (read_len > 0) {
                    buffer[read_len - 1] = 0;
                    printf("[%s:%d]", inet_ntoa(client->addr.sin_addr), htons(client->addr.sin_port));
                    printf(" %s\n", buffer);
                } else {
                    close(client->connfd);
                    printf("[%s:%d]", inet_ntoa(client->addr.sin_addr), htons(client->addr.sin_port));
                    cout << " Connection closed" << endl;
                    clients[i] = clients[active_clients - 1];
                    active_clients--;
                    i--;
                }
            }
        }
    }

    return 0;
}
