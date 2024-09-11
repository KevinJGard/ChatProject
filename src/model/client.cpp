#include "client.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
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
                        cout << "\rWelcome." << endl;
                        cout << "Message or /help: ";
                        cout.flush();
                    } else if (json_message["result"] == "USER_ALREADY_EXISTS") {
                        cout << "\rThe user already exists." << endl;
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }
                } else if (json_message["operation"] == "INVALID") {
                    if (json_message["result"] == "NOT_IDENTIFIED") {
                        cout << "\rYou're not identified." << endl;
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    } else if (json_message["result"] == "INVALID") {
                        cout << "\rOperation invalid." << endl;
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }
                }
            } else if (json_message["type"] == "NEW_USER") {
                string user = json_message["username"];
                cout << "\rNew client connected: " << user << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "NEW_STATUS") {
                string user = json_message["username"];
                string status = json_message["status"];
                cout << "\r" << user << " changed his status to " << status << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "USER_LIST") {
                cout << "\rUsers: " << json_message["users"] << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "PUBLIC_TEXT_FROM") {
                string user = json_message["username"];
                string text = json_message["text"]; 
                cout << "\r" << user << ": " << text << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else if (json_message["type"] == "DISCONNECTED") {
                string disconnected = json_message["username"];
                cout << "\r" << disconnected << " disconnected." << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else {
                cout << "\rServer: " << buffer << endl;
                cout << "Message or /help: ";
                cout.flush();
            }
        } else {
            cerr << "Error receiving data from server." << endl;
            break;
        }
    }
}