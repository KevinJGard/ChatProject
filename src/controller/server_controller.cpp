#include "server_controller.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

ServerController::ServerController(const string& ip, int port) : ip(ip), port(port), running(true), model(make_unique<ServerModel>()), view(make_unique<ServerView>()) {
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
        view->show_client_connection();
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
        close_sockfd("Invalid IP address.", sockfd);
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        close_sockfd("Error binding socket... ", sockfd);
        exit(EXIT_FAILURE);
    }
    if (listen(sockfd, 10) < 0) {
        close_sockfd("Failed to listen on socket.", sockfd);
        exit(EXIT_FAILURE);
    }
    view->show_server_run(ip, port);
}

void ServerController::handle_client(int client_sockfd) {
	char buffer[BUFFER_SIZE];
	int bytesRead = read(client_sockfd, buffer, BUFFER_SIZE - 1);
    string client_id;
	if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        string message = buffer;
        json identification;
        try {
            identification = json::parse(message);
            if (identification["type"] != "IDENTIFY") {
                json not_identify = {
                    {"type", "RESPONSE"},
                    {"operation", "INVALID"},
                    {"result", "NOT_IDENTIFIED"}
                };
                model->send_message(not_identify, client_sockfd);
                close(client_sockfd);
                return;
            }
            client_id = identification["username"];
            handle_identification(identification, client_id, client_sockfd);
        } catch (const json::parse_error& e) {
            json error_msg = {
                {"type", "RESPONSE"},
                {"operation", "INVALID"},
                {"result", "INVALID"}
            };
            model->send_message(error_msg, client_sockfd);
            close_sockfd("Error parsing JSON. Client disconnected.", client_sockfd);
            return;
        }
    } else {
        close_sockfd("Error receiving identification from client.", client_sockfd);
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
                    process_client_message(message, client_id, client_sockfd);
                } catch (const json::parse_error& e) {
                    json error_msg = {
                        {"type", "RESPONSE"},
                        {"operation", "INVALID"},
                        {"result", "INVALID"}
                    };
                    model->send_message(error_msg, client_sockfd);
                    cerr << "Error parsing JSON. Client disconnected." << endl;
                    break;
                }
            } else {
                cerr << "Error reading from client or client disconnected." << endl;
                handle_disconnect(client_id, client_sockfd);
                break;
            }
    }
    close(client_sockfd);
}

void ServerController::handle_identification(const json& identification, const string& client_id, int client_sockfd) {
    if (!model->add_user(client_id, client_sockfd)) {
        json identification_exists = {
            {"type", "RESPONSE"},
            {"operation", "IDENTIFY"},
            {"result", "USER_ALREADY_EXISTS"},
            {"extra", client_id}
        };
        model->send_message(identification_exists, client_sockfd);
        close(client_sockfd);
        return;
    }
    view->show_client_identification(client_id);
    json response_identify = {
        {"type", "RESPONSE"},
        {"operation", "IDENTIFY"},
        {"result", "SUCCESS"},
        {"extra", client_id}
    };
    model->send_message(response_identify, client_sockfd);
    json new_user_msg = {
        {"type", "NEW_USER"},
        {"username", client_id}
    };
    model->send_message_everyone(new_user_msg, client_id);
    user_status_map[client_id] = "ACTIVATE";
}

void ServerController::process_client_message(const string& message, const string& client_id, int client_sockfd) {
    json json_message = json::parse(message);
    if (json_message["type"] == "PUBLIC_TEXT") {
        json public_text = {
            {"type", "PUBLIC_TEXT_FROM"},
            {"username", client_id},
            {"text", json_message["text"]}
        };
        model->send_message_everyone(public_text, client_id);
    } else if (json_message["type"] == "STATUS") {
        json new_status_msg = {
            {"type", "NEW_STATUS"},
            {"username", client_id},
            {"status", json_message["status"]}
        };
        model->send_message_everyone(new_status_msg, client_id);
        user_status_map[client_id] = json_message["status"];
    } else if (json_message["type"] == "USERS") {
        json users_map = {
            {"type", "USER_LIST"},
            {"users", user_status_map}
        };
        model->send_message(users_map, client_sockfd);
    } else if (json_message["type"] == "TEXT") {
        string user = json_message["username"];
        if (user_status_map.find(user) != user_status_map.end()){
            json pvt_msg = {
                {"type", "TEXT_FROM"},
                {"username", client_id},
                {"text", json_message["text"]}
            };
            model->send_message_private(pvt_msg, user);
        } else {
            json pvt_msg_error = {
                {"type", "RESPONSE"},
                {"operation", "TEXT"},
                {"result", "NO_SUCH_USER"},
                {"extra", user}
            };
            model->send_message(pvt_msg_error, client_sockfd);
        }
    } else if (json_message["type"] == "DISCONNECT") {
        handle_disconnect(client_id, client_sockfd);
    } else {
        lock_guard<mutex> lock(mtx);
        cout << client_id << ": " << message << endl;
        string response = "message received";
        send(client_sockfd, response.c_str(), response.length(), 0);
    }
}

void ServerController::handle_disconnect(const string& client_id, int client_sockfd) {
    json disconnect_msg = {
        {"type", "DISCONNECTED"},
        {"username", client_id}
    };
    model->send_message_everyone(disconnect_msg, client_id);
    model->remove_user(client_id);
    view->show_client_disconnection(client_id);
}

void ServerController::close_sockfd(string message, int sockfd) {
    cerr << message << endl;
    close(sockfd);
}