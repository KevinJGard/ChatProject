#include "client_controller.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

ClientController::ClientController(const string& ip, int port) : model(ip, port) {}

void ClientController::run() {
	view.show_message("Connected to server");
	view.console("Enter your identification: ");
	string username;
    getline(cin, username);
    if (username.length() > 8) {
        view.show_message("\033[38;5;197mUsernames are limited to 8 characters.\033[0m");
        exit(EXIT_FAILURE);
    }
    json identification = {
	    {"type", "IDENTIFY"},
	    {"username", username}
	};
	model.send_message(identification.dump());
	receive_thread = thread(&ClientModel::receive_message, &model);
	process_input();
	if (receive_thread.joinable()) {
		receive_thread.join();
	}
}

void ClientController::process_input() {
	while (true) {
		view.console("Message or /help: ");
		string message;
        getline(cin, message);
        process_commands(message);
	}
}

void ClientController::process_commands(const string& command){
	if (command == "/help") {
        view.show_help();
    } else if (command == "/status_ACTIVATE") {
        json status_activate = {
            {"type", "STATUS"},
            {"status", "ACTIVATE"}
        };
        model.send_message(status_activate.dump());
    } else if (command == "/status_AWAY") {
        json status_away = {
            {"type", "STATUS"},
            {"status", "AWAY"}
        };
        model.send_message(status_away.dump());
    } else if (command == "/status_BUSY") {
        json status_busy = {
            {"type", "STATUS"},
            {"status", "BUSY"}
        };
        model.send_message(status_busy.dump());
    } else if (command == "/users") {
        json list = {
            {"type", "USERS"}
        };
        model.send_message(list.dump());
    } else if (command.rfind("/pvtmsg", 0) == 0) {
        size_t underscore_pos = command.find('_', 7);
        if (underscore_pos != string::npos) {
            size_t space_pos = command.find(' ', underscore_pos);
            if (space_pos != string::npos) {
                string username = command.substr(underscore_pos + 1, space_pos - underscore_pos - 1);
                string message = command.substr(space_pos + 1);
                json pvt_msg = {
                    {"type", "TEXT"},
                    {"username", username},
                    {"text", message}
                };
                model.send_message(pvt_msg.dump());
            } else {
                cout << "\033[38;5;197mInvalid command format. No message found.\033[0m" << endl;
            }
        } else {
            cout << "\033[38;5;197mInvalid command format.\033[0m" << endl;
        }
    } else if (command == "/exit") {
        json disconnect = {
            {"type", "DISCONNECT"}
        };
        model.send_message(disconnect.dump());
        exit(0);
    } else {
        json public_text = {
            {"type", "PUBLIC_TEXT"},
            {"text", command}
        };
        model.send_message(public_text.dump());
    }
}