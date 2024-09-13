#include "server_view.h"
#include <iostream>
using namespace std;

void ServerView::show_server_run(string ip, int port) {
	cout << "Server is listening on the IP " << ip << " and port " << port << " ..." << endl;
}

void ServerView::show_client_connection() {
	cout << "New client connected." << endl;
}

void ServerView::show_client_identification(const string& client_id) {
	cout << "Client identified as: " << client_id << endl;
}

void ServerView::show_client_disconnection(const string& client_id) {
	cout << "Client " << client_id << " disconnected." << endl;
}