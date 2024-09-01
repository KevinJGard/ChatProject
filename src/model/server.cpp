#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex>

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
    const int BUFFER_SIZE = 256;
    bool running;
    mutex mtx;

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
            client_id = buffer;
            lock_guard<mutex> lock(mtx);
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
                if (message == "/exit") {
                    lock_guard<mutex> lock(mtx);
                    cout << "Client " << client_id << " disconnected." << endl;
                    break;
                }
                lock_guard<mutex> lock(mtx);
                cout << client_id << ": " << message << endl;
                string response = "message received";
                send(client_sockfd, response.c_str(), response.size(), 0);
            } else {
                cerr << "Error reading from client or client disconnected." << endl;
                break;
            }
        }
        close(client_sockfd);
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