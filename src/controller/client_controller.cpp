#include "client_controller.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

ClientController::ClientController(const string& ip, int port) : model(make_unique<ClientModel>(ip, port)), view(make_unique<ClientView>()) {}

void ClientController::run() {
	view->show_message("Connected to server");
	view->console("Enter your identification: ");
	string username;
    getline(cin, username);
    if (username.length() > 8) {
        view->show_message("\033[38;5;197mUsernames are limited to 8 characters.\033[0m");
        exit(EXIT_FAILURE);
    } else if (model->contain_spaces(username)) {
        view->show_message("\033[38;5;197mPlease do not use space in the username.\033[0m");
        exit(EXIT_FAILURE);
    }
    json identification = {
	    {"type", "IDENTIFY"},
	    {"username", username}
	};
	model->send_message(identification.dump());
	receive_thread = thread(&ClientController::receive_messages, this);
	process_input();
	if (receive_thread.joinable()) {
		receive_thread.join();
	}
}

void ClientController::process_input() {
	while (true) {
		view->console("Message or /help: ");
		string message;
        getline(cin, message);
        process_commands(message);
	}
}

void ClientController::process_commands(const string& command) {
	if (command == "/help") {
        view->show_help();
    } else if (command == "/status_ACTIVATE") {
        process_status("ACTIVATE");
    } else if (command == "/status_AWAY") {
        process_status("AWAY");
    } else if (command == "/status_BUSY") {
        process_status("BUSY");
    } else if (command == "/users") {
        json list = {
            {"type", "USERS"}
        };
        model->send_message(list.dump());
    } else if (command.rfind("/pvtmsg", 0) == 0) {
        process_private_message(command);
    } else if (command == "/exit") {
        json disconnect = {
            {"type", "DISCONNECT"}
        };
        model->send_message(disconnect.dump());
        exit(0);
    } else {
        json public_text = {
            {"type", "PUBLIC_TEXT"},
            {"text", command}
        };
        model->send_message(public_text.dump());
    }
}

void ClientController::process_status(const string& status) {
    json status_json = {
        {"type", "STATUS"},
        {"status", status}
    };
    model->send_message(status_json.dump());
}

void ClientController::process_private_message(const string& command) {
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
            model->send_message(pvt_msg.dump());
        } else {
            view->show_message("\033[38;5;197mInvalid command format. No message found.\033[0m");
        }
    } else {
        view->show_message("\033[38;5;197mInvalid command format.\033[0m");
    }
}

void ClientController::handle_error(const string& error_message) {
    cout << "\r" << string(50, ' ') << "\r";
    view->show_message(pink_color + error_message + color_reset);
    close(model->get_sockfd());
    exit(EXIT_FAILURE);
}

void ClientController::handle_user_status(const json& json_message) {
    string user = json_message["username"];
    int color = model->get_user_color(user);
    string status = json_message["status"];
    string color_status;
    if (status == "ACTIVATE"){
        color_status = green_color;
    } else if (status == "AWAY") {
        color_status = yellow_color;
    } else if (status == "BUSY") {
        color_status = red_color;
    }
    view->show_default(color_rdm + to_string(color) + "m" + user + color_reset + " changed his status to " + color_status + status + color_reset);
}

void ClientController::handle_msg_user(const json& json_message) {
    string user = json_message["username"];
    string text = json_message["text"]; 
    int color = model->get_user_color(user);
    string pvt;
    if (json_message["type"] == "PUBLIC_TEXT_FROM") {
        pvt = "";
    } else {
        pvt = pvt_red_color;
    }
    view->show_default(pvt + color_rdm + to_string(color) + "m" + user + ": " + color_reset + text);
}

void ClientController::handle_disconnection(const json& json_message) {
    string user = json_message["username"];
    int color = model->get_user_color(user);
    view->show_default(color_rdm + to_string(color) + "m" + user + color_reset + red_color + " disconnected." + color_reset);
}

void ClientController::handle_response(const json& json_message) {
    if (json_message["operation"] == "IDENTIFY") {
        if (json_message["result"] == "SUCCESS") {
            view->show_default(blue_color + "Welcome." + color_reset);
        } else if (json_message["result"] == "USER_ALREADY_EXISTS") {
            handle_error("The user already exists.");
        }
    } else if (json_message["operation"] == "INVALID") {
        if (json_message["result"] == "NOT_IDENTIFIED") {
            handle_error("You're not identified.");
        } else if (json_message["result"] == "INVALID") {
            handle_error("Operation invalid.");
        }
    } else if (json_message["operation"] == "TEXT") {
        view->show_default(pink_color + "No such user." + color_reset);
    }
}

void ClientController::receive_messages() {
    while (true) {
        string message = model->receive_message();
        process_receive_messages(message);
    }
}

void ClientController::process_receive_messages(const string& message) {
    json json_message;
    try {
        json_message = json::parse(message);
    } catch (const json::parse_error& e) {
        view->show_message("Error parsing JSON. Client disconnected.");
        close(model->get_sockfd());
        exit(EXIT_FAILURE);
    }
    if (json_message["type"] == "RESPONSE") {
        handle_response(json_message);
    } else if (json_message["type"] == "NEW_USER") {
        string user = json_message["username"];
        int color = model->get_user_color(user);
        view->show_default(green_color + "New client connected: " + color_reset + color_rdm + to_string(color) + "m" + user + color_reset);
    } else if (json_message["type"] == "NEW_STATUS") {
        handle_user_status(json_message);
    } else if (json_message["type"] == "USER_LIST") {
        view->show_default("Users: " + json_message["users"].dump());
    } else if (json_message["type"] == "PUBLIC_TEXT_FROM") {
        handle_msg_user(json_message);
    } else if (json_message["type"] == "TEXT_FROM") {
        handle_msg_user(json_message);
    } else if (json_message["type"] == "DISCONNECTED") {
        handle_disconnection(json_message);
    } else {
        view->show_default("Server: " + message);
    }
}