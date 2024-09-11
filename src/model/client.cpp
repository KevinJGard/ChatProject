#include "client.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
using namespace std;

ClientModel::ClientModel(const string& ip, int port) : ip(ip), port(port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cout << "Failed to create socket." << endl;
        exit(EXIT_FAILURE);
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        cout << "Invalid IP address." << endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cout << "Error connecting to server..." << endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

ClientModel::~ClientModel() {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

void ClientModel::send_message(const string& message) {
    send(sockfd, message.c_str(), message.length(), 0);
}

void ClientModel::receive_message() {
    char buffer[BUFFER_SIZE];
    while (true) {
        int n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            cout << "\rServer: " << buffer << endl;
            cout << "Message or /help: ";
            cout.flush();
        } else {
            cerr << "Error receiving data from server." << endl;
            break;
        }
    }
}