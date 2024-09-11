#ifndef SERVER_CONTROLLER_H
#define SERVER_CONTROLLER_H

#include "../model/server.h"
#include "../view/server_view.h"
#include <thread>
#include <mutex>
#include <unordered_map>
#include <netinet/in.h>
using namespace std;

class ServerController {
public:
	ServerController(const string& ip, int port);
	~ServerController();
	void run();
	void stop();

private:
	void setup_server();
	void handle_client(int client_sockfd);

	string ip;
	int port; 
	int sockfd;
	const int BUFFER_SIZE = 1024;
	bool running;
	ServerModel model;
	ServerView view;
	mutex mtx;
	unordered_map<string, string> user_status_map;
};

#endif