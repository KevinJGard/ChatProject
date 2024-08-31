#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <unistd.h>
using namespace std;

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		cout << "Failed to create socket." << endl;
		exit(1);
	}

	cout << "Server Socket connection created..." << endl;

	sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(1234);

    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
    	cout << "Error binding socket... " << endl;
    	exit(1);
    }

    if (listen(sockfd, 10) < 0) {
    	cout << "Failed to listen on socket." << endl;
    	exit(1);
    }

    auto addrlen = sizeof(sockaddr);
    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
    if (connection < 0) {
    	cout << "Failed to grab connection." << endl;
    	exit(1);
    }

    char buffer[100];
    auto bytesRead = read(connection, buffer, 100);
    cout << "The message was: " << buffer;

    string response = "Hello from Server\n";
    send(connection, response.c_str(), response.size(), 0);

    close(connection);
    close(sockfd);
}