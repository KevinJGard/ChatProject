#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <termios.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

class Client {
public:
    Client(int port) : port(port) {
        setup_client();
    }

    ~Client() {
        if (sockfd >= 0) {
            close(sockfd);
        }
    }

    void run() {
        cout << "Connected to server" << endl;
        cout << "Enter your identification: ";
        string username;
        getline(cin, username);
        json identification = {
            {"type", "IDENTIFY"},
            {"username", username}
        };
        send_message(identification);

        thread receive_thread(&Client::receive_message, this);

        while (true) {
            cout << "Message or /help: ";
            string message;
            getline(cin, message);

            if (message == "/help") {
                cout << "Type /status_ACTIVATE to set your status to activated" << endl;
                cout << "Type /status_AWAY to set your status to away" << endl;
                cout << "Type /status_BUSY to set your status to busy" << endl;
                cout << "Type /users to see the list of users in the chat" << endl;
                cout << "Type /exit to disconnect from the chat"<< endl;
                continue;
            } 
            if (message == "/status_ACTIVATE") {
                json status_activate = {
                    {"type", "STATUS"},
                    {"status", "ACTIVATE"}
                };
                send_message(status_activate);
            } else if (message == "/status_AWAY") {
                json status_away = {
                    {"type", "STATUS"},
                    {"status", "AWAY"}
                };
                send_message(status_away);
            } else if (message == "/status_BUSY") {
                json status_busy = {
                    {"type", "STATUS"},
                    {"status", "BUSY"}
                };
                send_message(status_busy);
            } else if (message == "/users") {
                json list = {
                    {"type", "USERS"}
                };
                send_message(list);
            } else if (message == "/exit") {
                json exit = {
                    {"type", "DISCONNECT"}
                };
                send_message(exit);
                break;
            } else {
                json public_text = {
                    {"type", "PUBLIC_TEXT"},
                    {"text", message}
                };
                send_message(public_text);
            }
        }
        if (receive_thread.joinable()) 
            receive_thread.join();
    }

private:
    int port;
    int sockfd;
    const int BUFFER_SIZE = 1024;
    mutex mtx;

    void setup_client() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            cout << "Failed to create socket." << endl;
            exit(EXIT_FAILURE);
        }
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cout << "Error connecting to server..." << endl;
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    void send_message(const json& message) {
        string msg = message.dump();
        send(sockfd, msg.c_str(), msg.length(), 0);
    }

    void receive_message() {
        while (true) {
            char buffer[BUFFER_SIZE] = {0};
            int n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
            if (n > 0) {
                buffer[n] = '\0';
                lock_guard<mutex> lock(mtx);
                cout << "\rServer: " << buffer << endl;
                cout << "Message or /help: ";
                cout.flush();
            } else {
                cerr << "Error receiving data from server." << endl;
                break;
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
    Client client(port);
    client.run();

    return 0;
}