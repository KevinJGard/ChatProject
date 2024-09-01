#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
using namespace std;

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

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

    cout << "Connected to server" << endl;
    cout << "Enter your identification: ";

    string identification;
    getline(cin, identification);
    send(sockfd, identification.c_str(), identification.length(), 0);

    while (true) {
        cout << "Enter your message: ";
        string message;
        getline(cin, message);
        if (message == "/exit") {
            send(sockfd, message.c_str(), message.length(), 0);
            break;
        }
        send(sockfd, message.c_str(), message.length(), 0);
        char buffer[256] = {0};
        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            cout << "Server: " << buffer << endl;
        } else {
            cerr << "Error receiving data from server." << endl;
            break;
        }
    }

    close(sockfd);
    return 0;
}