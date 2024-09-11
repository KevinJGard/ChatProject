#ifndef CLIENT_MODEL_H
#define CLIENT_MODEL_H

#include <string>
using namespace std;

class ClientModel {
public:
	ClientModel(const string& ip, int port);
	~ClientModel();

	void send_message(const string& message);
	void receive_message();

private:
	string ip;
    int port;
    int sockfd;
    const int BUFFER_SIZE = 1024;
};

#endif