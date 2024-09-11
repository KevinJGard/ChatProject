#include "client_controller.h"
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

ClientController::ClientController(int port) : model(port) {}

void ClientController::run() {
	view.show_message("Connected to server");
	view.console("Enter your identification: ");
	string username;
    getline(cin, username);
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