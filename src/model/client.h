#ifndef CLIENT_MODEL_H
#define CLIENT_MODEL_H

#include <string>
#include <unordered_map>
using namespace std;

class ClientModel {
public:
	ClientModel(const string& ip, int port);
	~ClientModel();

	void send_message(const string& message);
	string receive_message();
	bool contain_spaces(const string& username);
	int get_sockfd();
	int get_user_color(const string& user);

private:
	int generate_random_value();
	string ip;
    int port;
    int sockfd;
    const int BUFFER_SIZE = 1024;
    unordered_map<string, int> user_color_map;
};

#endif