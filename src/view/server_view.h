#ifndef SERVER_VIEW_H
#define SERVER_VIEW_H

#include <string>
using namespace std;

/**
 * View component responsible for displaying server status and client actions.
 */
class ServerView {
public:
	/**
	 * Show that the server is running and listening for connections.
	 */
	void show_server_run(string ip, int port);
	/**
	 * Display a message when a new client connects.
	 */
	void show_client_connection();
	/**
	 * Show the client’s identification once it is received.
	 */
	void show_client_identification(const string& client_id);
	/**
	 * Show the client disconnection message.
	 */
	void show_client_disconnection(const string& client_id);
};

#endif