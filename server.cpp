#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <vector>

#define PORT 8080
#define BUFFER_SIZE 1024

std::vector<int> clients;       // connected client sockets
pthread_mutex_t clients_mutex;  // to protect shared vector

void broadcastMessage(const std::string &message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int client : clients) {
        if (client != sender_sock) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handleClient(void *arg) {
    int client_sock = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            std::cout << "Client disconnected\n";
            close(client_sock);

            pthread_mutex_lock(&clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client_sock), clients.end());
            pthread_mutex_unlock(&clients_mutex);

            break;
        }

        std::string msg(buffer);
        std::cout << "Client: " << msg;

        broadcastMessage(msg, client_sock);
    }

    return nullptr;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    pthread_mutex_init(&clients_mutex, nullptr);

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen
    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        return 1;
    }

    std::cout << "Server started on port " << PORT << "\n";

    while (true) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        clients.push_back(client_sock);
        pthread_mutex_unlock(&clients_mutex);

        pthread_t tid;
        pthread_create(&tid, nullptr, handleClient, &client_sock);
        pthread_detach(tid);

        std::cout << "New client connected\n";
    }

    close(server_sock);
    pthread_mutex_destroy(&clients_mutex);

    return 0;
}

