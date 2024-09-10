#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

class Server {
public:
    Server(int port) : port(port), running(true) {
        setup_server();
    }

    ~Server() {
        stop();
    }

    void run() {
        while (running) {
            sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int new_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
            if (new_sockfd < 0) {
                cerr << "Failed to accept new connection." << endl;
                continue;
            }
            cout << "New client connected." << endl;
            thread client_thread(&Server::handle_client, this, new_sockfd);
            client_thread.detach();
        }
    }

    void stop() {
        running = false;
        close(sockfd);
    }

private:
    int port, sockfd;
    const int BUFFER_SIZE = 1024;
    bool running;
    mutex mtx;
    unordered_set<string> usernames;
    unordered_map<string, int> user_socket_map;

    void setup_server() {
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
        cout << "Server is listening on port " << port << " ..." << endl;
    }

    void handle_client(int client_sockfd) {
        char buffer[BUFFER_SIZE];
        int bytesRead;
        string client_id;

        bytesRead = read(client_sockfd, buffer, BUFFER_SIZE - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            string message = buffer;
            json identification = json::parse(message);
            client_id = identification["username"];
            lock_guard<mutex> lock(mtx);
            if (usernames.find(client_id) != usernames.end()) {
                json identification_exits = {
                    {"type", "RESPONSE"},
                    {"operation", "IDENTIFY"},
                    {"result", "USER_ALREADY_EXISTS"},
                    {"extra", client_id}
                };
                string msg = identification_exits.dump();
                send(client_sockfd, msg.c_str(), msg.length(), 0);
                close(client_sockfd);
                return;
            }
            usernames.insert(client_id);
            user_socket_map[client_id] = client_sockfd;
            cout << "Client identified as: " << client_id << endl;
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
                    message_everyone(public_text, client_id);
                } else if (json_message["type"] == "DISCONNECT") {
                    lock_guard<mutex> lock(mtx);
                    usernames.erase(client_id);
                    user_socket_map.erase(client_id);
                    cout << "Client " << client_id << " disconnected." << endl;
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

    void message_everyone(const json& message, const string& user) {
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
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        cerr << "Invalid port number. Port must be between 1 and 65535." << endl;
        exit(EXIT_FAILURE);
    }
    Server server(port);
    server.run();

    return 0;
}