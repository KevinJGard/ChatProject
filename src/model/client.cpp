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

void ClientModel::receive_message() {
    char buffer[BUFFER_SIZE];
    while (true) {
        int n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            string message = buffer;
            json json_message;
            try {
                json_message = json::parse(message);
            } catch (const json::parse_error& e) {
                cerr << "Error parsing JSON. Client disconnected." << endl;
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            if (json_message["type"] == "RESPONSE") {
                if (json_message["operation"] == "IDENTIFY") {
                    if (json_message["result"] == "SUCCESS") {
                        cout << "\r" << string(50, ' ') << "\r";
                        cout << "\033[38;5;30mWelcome.\033[0m" << endl;
                        cout << "Message or /help: ";
                        cout.flush();
                    } else if (json_message["result"] == "USER_ALREADY_EXISTS") {
                        cout << "\r" << string(50, ' ') << "\r";
                        cout << "\033[38;5;197mThe user already exists. \033[0m" << endl;
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }
                } else if (json_message["operation"] == "INVALID") {
                    if (json_message["result"] == "NOT_IDENTIFIED") {
                        cout << "\r" << string(50, ' ') << "\r";
                        cout << "\033[38;5;197mYou're not identified.\033[0m" << endl;
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    } else if (json_message["result"] == "INVALID") {
                        cout << "\r" << string(50, ' ') << "\r";
                        cout << "\033[38;5;197mOperation invalid.\033[0m" << endl;
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }
                } else if (json_message["operation"] == "TEXT") {
                    cout << "\r" << string(50, ' ') << "\r";
                    cout << "\033[38;5;197mNo such user.\033[0m" << endl;
                    cout << "Message or /help: ";
                    cout.flush();
                }
            } else if (json_message["type"] == "NEW_USER") {
                string user = json_message["username"];
                int color = get_user_color(user);
                cout << "\r" << string(50, ' ') << "\r";
                cout << "\033[38;5;40mNew client connected: \033[0m" << "\033[38;5;" << color << "m" << user << "\033[0m" << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "NEW_STATUS") {
                string user = json_message["username"];
                int color = get_user_color(user);
                string status = json_message["status"];
                cout << "\r" << string(50, ' ') << "\r";
                if (status == "ACTIVATE"){
                    cout << "\033[38;5;" << color << "m" << user << "\033[0m" << " changed his status to " << "\033[38;5;40m" << status << "\033[0m" << endl;
                } else if (status == "AWAY") {
                    cout << "\033[38;5;" << color << "m" << user << "\033[0m" << " changed his status to " << "\033[38;5;190m" << status << "\033[0m" << endl;
                } else if (status == "BUSY") {
                    cout << "\033[38;5;" << color << "m" << user << "\033[0m" << " changed his status to " << "\033[38;5;160m" << status << "\033[0m" << endl;
                }
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "USER_LIST") {
                cout << "\r" << string(50, ' ') << "\r";
                cout << "Users: " << json_message["users"] << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "PUBLIC_TEXT_FROM") {
                string user = json_message["username"];
                string text = json_message["text"]; 
                int color = get_user_color(user);
                cout << "\r" << string(50, ' ') << "\r";
                cout << "\033[38;5;" << color << "m" << user << ": \033[0m" << text << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "TEXT_FROM") {
                string user = json_message["username"];
                string text = json_message["text"];
                int color = get_user_color(user);
                cout << "\r" << string(50, ' ') << "\r";
                cout << "\033[38;5;160mPrivate \033[0m";
                cout << "\033[38;5;" << color << "m" << user << ": \033[0m" << text << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "DISCONNECTED") {
                string user = json_message["username"];
                int color = get_user_color(user);
                cout << "\r" << string(50, ' ') << "\r";
                cout << "\033[38;5;" << color << "m" << user << "\033[0m" << " \033[38;5;160mdisconnected.\033[0m" << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else {
                cout << "\r" << string(50, ' ') << "\r";
                cout << "Server: " << buffer << endl;
                cout << "Message or /help: ";
                cout.flush();
            }
        } else {
            cerr << "Error receiving data from server." << endl;
            break;
        }
    }
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

bool ClientModel::contain_spaces(const string& username) {
    for (char c : username) {
        if (c == ' ') {
            return true;
        }
    }
    return false;
}