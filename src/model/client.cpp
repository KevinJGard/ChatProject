#include "client.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <random>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
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

string ClientModel::receive_message() {
    char buffer[BUFFER_SIZE];
    int n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        string message = buffer;
        return message;
    }
    return "Error receiving data from server.";
}

bool ClientModel::contain_spaces(const string& username) {
    for (char c : username) {
        if (c == ' ') {
            return true;
        }
    }
    return false;
}

int ClientModel::get_sockfd() {
    return sockfd;
}

int ClientModel::generate_random_value() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0,255);
    int num = dis(gen);
    return num;
}

int ClientModel::get_user_color(const string& user) {
    auto it = user_color_map.find(user);
    if (it != user_color_map.end()) {
        return it -> second;
    } else {
        int color = generate_random_value();
        user_color_map[user] = color;
        return color;
    }
}
