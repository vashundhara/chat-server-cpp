#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sock;

void *receiveMessages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cout << "Disconnected from server\n";
            close(sock);
            exit(0);
        }
        std::cout << "Message: " << buffer;
    }
    return nullptr;
}

int main() {
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // local server

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    std::cout << "Connected to server\n";

    pthread_t tid;
    pthread_create(&tid, nullptr, receiveMessages, nullptr);

    std::string msg;
    while (true) {
        std::getline(std::cin, msg);
        send(sock, msg.c_str(), msg.size(), 0);
    }

    close(sock);
    return 0;
}

