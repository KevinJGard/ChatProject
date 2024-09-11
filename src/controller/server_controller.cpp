#include "server_controller.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

ServerController::ServerController(const string& ip, int port) : ip(ip), port(port), running(true) {
	setup_server();
}

ServerController::~ServerController() {
	stop();
}

void ServerController::run() {
	while (running) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int new_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (new_sockfd < 0) {
            cerr << "Failed to accept new connection." << endl;
            continue;
        }
        view.show_client_connection();
        thread client_thread(&ServerController::handle_client, this, new_sockfd);
        client_thread.detach();
    }
}

void ServerController::stop() {
	running = false;
    close(sockfd);
}

void ServerController::setup_server() {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cerr << "Failed to create socket." << endl;
        exit(EXIT_FAILURE);
    }
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &sockaddr.sin_addr) <= 0) {
        cout << "Invalid IP address." << endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        cerr << "Error binding socket... " << endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (listen(sockfd, 10) < 0) {
        cerr << "Failed to listen on socket." << endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    view.show_server_run(port);
}

void ServerController::handle_client(int client_sockfd) {
	char buffer[BUFFER_SIZE];
	int bytesRead;
    string client_id;
	bytesRead = read(client_sockfd, buffer, BUFFER_SIZE - 1);
	if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        string message = buffer;
        json identification;
        try {
            identification = json::parse(message);
            client_id = identification["username"];
        } catch (const json::parse_error& e) {
            json error_msg = {
                {"type", "RESPONSE"},
                {"operation", "INVALID"},
                {"result", "INVALID"}
            };
            string msg = error_msg.dump();
            send(client_sockfd, msg.c_str(), msg.length(), 0);
            cerr << "Error parsing JSON. Client disconnected." << endl;
            close(client_sockfd);
            return;
        }
        if (!model.add_user(client_id, client_sockfd)) {
            json identification_exists = {
                {"type", "RESPONSE"},
                {"operation", "IDENTIFY"},
                {"result", "USER_ALREADY_EXISTS"},
                {"extra", client_id}
            };
            string msg = identification_exists.dump();
            send(client_sockfd, msg.c_str(), msg.length(), 0);
            close(client_sockfd);
            return;
        }
        view.show_client_identification(client_id);
        json response_identify = {
            {"type", "RESPONSE"},
            {"operation", "IDENTIFY"},
            {"result", "SUCCESS"},
            {"extra", client_id}
        };
        string msg_success = response_identify.dump();
        send(client_sockfd, msg_success.c_str(), msg_success.length(), 0);
        json new_user_msg = {
            {"type", "NEW_USER"},
            {"username", client_id}
        };
        model.message_everyone(new_user_msg, client_id);
        user_status_map[client_id] = "ACTIVATE";
    } else {
        cerr << "Error receiving identification from client." << endl;
        close(client_sockfd);
        return;
    }
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
            bytesRead = read(client_sockfd, buffer, BUFFER_SIZE - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                string message = buffer;
                json json_message;
                try {
                    json_message = json::parse(message);
                } catch (const json::parse_error& e) {
                    json error_msg = {
                        {"type", "RESPONSE"},
                        {"operation", "INVALID"},
                        {"result", "INVALID"}
                    };
                    string msg = error_msg.dump();
                    send(client_sockfd, msg.c_str(), msg.length(), 0);
                    cerr << "Error parsing JSON. Client disconnected." << endl;
                    break;
                }
                if (json_message["type"] == "PUBLIC_TEXT") {
                    json public_text = {
                        {"type", "PUBLIC_TEXT_FROM"},
                        {"username", client_id},
                        {"text", json_message["text"]}
                    };
                    model.message_everyone(public_text, client_id);
                } else if (json_message["type"] == "STATUS") {
                    json new_status_msg = {
                        {"type", "NEW_STATUS"},
                        {"username", client_id},
                        {"status", json_message["status"]}
                    };
                    model.message_everyone(new_status_msg, client_id);
                    user_status_map[client_id] = json_message["status"];
                } else if (json_message["type"] == "USERS") {
                    json users_map = {
                        {"type", "USER_LIST"},
                        {"users", user_status_map}
                    };
                    string msg = users_map.dump();
                    send(client_sockfd, msg.c_str(), msg.length(), 0);
                } else if (json_message["type"] == "DISCONNECT") {
                    json disconnect_msg = {
                        {"type", "DISCONNECTED"},
                        {"username", client_id}
                    };
                    model.message_everyone(disconnect_msg, client_id);
                    model.remove_user(client_id);
                    view.show_client_disconnection(client_id);
                    break;
                } else {
                    lock_guard<mutex> lock(mtx);
                    cout << client_id << ": " << message << endl;
                    string response = "message received";
                    send(client_sockfd, response.c_str(), response.size(), 0);
                }
            } else {
                cerr << "Error reading from client or client disconnected." << endl;
                break;
            }
    }
    close(client_sockfd);
}
