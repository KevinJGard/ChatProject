#include "server_controller.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

ServerController::ServerController(int port) : port(port), running(true) {
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
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(port);
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
        json identification = json::parse(message);
        client_id = identification["username"];
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
                json json_message = json::parse(message);
                if (json_message["type"] == "PUBLIC_TEXT") {
                    json public_text = {
                        {"type", "PUBLIC_TEXT_FROM"},
                        {"username", client_id},
                        {"text", json_message["text"]}
                    };
                    model.message_everyone(public_text, client_id);
                } else if (json_message["type"] == "DISCONNECT") {
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
