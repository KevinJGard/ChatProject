#ifndef CLIENT_VIEW_H
#define CLIENT_VIEW_H

#include <string>
using namespace std;

class ClientView {
public:
	void show_message(const string& message);
	void console(const string& message);
	void show_help();
	void show_default(const string& message);
};

#endif