#include "client_view.h"
#include <iostream>
using namespace std;

void ClientView::show_message(const string& message) {
	cout << message << endl;
}

void ClientView::console(const string& message) {
    cout << message;
}

void ClientView::show_help() {
	cout << "Type /status_ACTIVE to set your status to activated." << endl;
    cout << "Type /status_AWAY to set your status to away." << endl;
    cout << "Type /status_BUSY to set your status to busy." << endl;
    cout << "Type /users to see the list of users in the chat." << endl;
    cout << "Type /pvtmsg_<username> <message> to send a private message to another user." << endl;
    cout << "Type /exit to disconnect from the chat."<< endl;
}

void ClientView::show_default(const string& message) {
    cout << "\r" << string(50, ' ') << "\r";
    cout << message << endl;
    cout << "Message or /help: ";
    cout.flush();
}