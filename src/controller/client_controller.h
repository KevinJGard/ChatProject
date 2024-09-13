#ifndef CLIENT_CONTROLLER_H
#define CLIENT_CONTROLLER_H

#include "../model/client.h"
#include "../view/client_view.h"
#include <thread>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

class ClientController {
public:
	ClientController(const string& ip, int port);
	void run();

private:
	void process_input();
	void process_commands(const string& command);
	void process_status(const string& status);
	void process_private_message(const string& command);
	void handle_error(const string& error_message);
	void handle_user_status(const json& json_message);
	void handle_msg_user(const json& json_message);
	void handle_disconnection(const json& json_message);
	void handle_response(const json& json_message);
	void receive_messages();
	void process_receive_messages(const string& message);

	ClientModel model;
	ClientView view;
	thread receive_thread;
	const string color_reset = "\033[0m";
	const string color_rdm = "\033[38;5;";
	const string pink_color = "\033[38;5;197m";
	const string blue_color = "\033[38;5;30m";
	const string green_color = "\033[38;5;40m";
	const string yellow_color = "\033[38;5;190m";
	const string red_color = "\033[38;5;160m";
	const string pvt_red_color = "\033[38;5;160mPrivate \033[0m";
};

#endif