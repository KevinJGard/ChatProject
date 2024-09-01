#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
using namespace std;

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cout << "Failed to create socket." << endl;
        exit(1);
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(12345);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(sockaddr)) < 0) {
        cout << "Error connecting to server..." << endl;
        exit(1);
    }

    cout << "Connected to server" << endl;

    const char* message = "Hello, server!";
    send(sockfd, message, strlen(message), 0);

    char buffer[256];
    int n = recv(sockfd, buffer, 255, 0);
    if (n > 0) {
        buffer[n] = '\0';
        cout << "Respuesta del servidor: " << buffer << endl;
    } else {
        cerr << "Error al recibir datos del servidor" << endl;
    }

    close(sockfd);
    return 0;
}