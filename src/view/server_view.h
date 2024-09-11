#ifndef SERVER_VIEW_H
#define SERVER_VIEW_H

#include <string>
using namespace std;

class ServerView {
public:
	void show_server_run(int port);
	void show_client_connection();
	void show_client_identification(const string& client_id);
	void show_client_disconnection(const string& client_id);
};

#endif