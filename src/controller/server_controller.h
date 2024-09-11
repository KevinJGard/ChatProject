#ifndef SERVER_CONTROLLER_H
#define SERVER_CONTROLLER_H

#include "../model/server.h"
#include "../view/server_view.h"
#include <thread>
#include <mutex>
#include <netinet/in.h>
using namespace std;

class ServerController {
public:
	ServerController(int port);
	~ServerController();
	void run();
	void stop();

private:
	void setup_server();
	void handle_client(int client_sockfd);

	int port, sockfd;
	const int BUFFER_SIZE = 1024;
	bool running;
	ServerModel model;
	ServerView view;
	mutable mutex mtx;
};

#endif