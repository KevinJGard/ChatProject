#include "server.h"
#include <iostream>

using namespace std;

bool ServerModel::add_user(const string& client_id, int client_sockfd) {
    lock_guard<mutex> lock(mtx);
    if(user_socket_map.find(client_id) != user_socket_map.end()) {
        return false;
    }
    user_socket_map[client_id] = client_sockfd;
    return true;
}

void ServerModel::remove_user(const string& client_id) {
    lock_guard<mutex> lock(mtx);
    user_socket_map.erase(client_id);
}

void ServerModel::send_message(const json& message, int client_sockfd) {
    string msg = message.dump();
    send(client_sockfd, msg.c_str(), msg.length(), 0);
}

void ServerModel::send_message_everyone(const json& message, const string& user) {
    string msg = message.dump();
    lock_guard<mutex> lock(mtx);
    for (unordered_map<string, int>::iterator it = user_socket_map.begin(); it != user_socket_map.end(); ++it) {
        const string& username = it->first;
        int client_sockfd = it->second;
        if (username != user) {
            if (send(client_sockfd, msg.c_str(), msg.length(), 0) < 0) {
                cerr << "Error sending message to client " << username << endl;
            }
        }
    }
}

void ServerModel::send_message_private(const json& message, const string& user) {
    string msg = message.dump();
    lock_guard<mutex> lock(mtx);
    auto it = user_socket_map.find(user);
    if (it != user_socket_map.end()) {
        int client_sockfd = it -> second;
        if (send(client_sockfd, msg.c_str(), msg.length(), 0) < 0) {
            cerr << "Error sending message to client " << user << endl;
        }
    }
}