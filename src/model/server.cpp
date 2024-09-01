#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
using namespace std;

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		cerr << "Failed to create socket." << endl;
		exit(EXIT_FAILURE);
	}

	sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(12345);

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
    cout << "Server is listening on port 12345..." << endl;

    auto addrlen = sizeof(sockaddr);
    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
    if (connection < 0) {
    	cerr << "Failed to grab connection." << endl;
    	exit(1);
    }

    string response = "Hello from Server\n";
    send(connection, response.c_str(), response.size(), 0);

    char buffer[256] = {0};
    auto bytesRead = read(connection, buffer, sizeof(buffer) - 1);
    cout << "The message was: " << buffer << endl;

    close(connection);
    close(sockfd);
    return 0;
}