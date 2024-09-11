#ifndef CLIENT_CONTROLLER_H
#define CLIENT_CONTROLLER_H

#include "../model/client.h"
#include "../view/client_view.h"
#include <thread>
using namespace std;

class ClientController {
public:
	ClientController(int port);
	void run();

private:
	void process_input();
	void process_commands(const string& command);

	ClientModel model;
	ClientView view;
	thread receive_thread;
};

#endif