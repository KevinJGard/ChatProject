#ifndef SERVER_CONTROLLER_H
#define SERVER_CONTROLLER_H

#include "../model/server.h"
#include "../view/server_view.h"
#include <thread>
#include <mutex>
#include <unordered_map>
#include <netinet/in.h>
#include <memory>
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
    void handle_identification(const json& identification, const string& client_id, int client_sockfd);
	void process_client_message(const string& message, const string& client_id, int client_sockfd);
    void handle_disconnect(const string& client_id, int client_sockfd);
    void close_sockfd(string message, int sockfd);

	string ip;
	int port; 
	int sockfd;
	const int BUFFER_SIZE = 1024;
	bool running;
	unique_ptr<ServerModel> model;
	unique_ptr<ServerView> view;
	mutex mtx;
	unordered_map<string, string> user_status_map;
};

#endif