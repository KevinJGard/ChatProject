#ifndef CLIENT_VIEW_H
#define CLIENT_VIEW_H

#include <string>
using namespace std;

/**
 * Class that handles message presentation and client user interface.
 */
class ClientView {
public:
	/**
	 * Displays a message to the console.
	 * @param message The message to display.
	 */
	void show_message(const string& message);
	/**
	 * Displays a message to the console without a newline.
	 * @param message The message to display.
	 */
	void console(const string& message);
	/**
	 * Displays commands to the console.
	 */
	void show_help();
	/**
	 * Displays a default message, overwriting the current line in the console.
	 * @param message The message to display.
	 */
	void show_default(const string& message);
};

#endif