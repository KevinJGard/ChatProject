#ifndef CLIENT_MODEL_H
#define CLIENT_MODEL_H

#include <string>
#include <unordered_map>
using namespace std;

/**
 * Class that handles client-server communication.
 */
class ClientModel {
public:
	/**
	 * Constructor for the class.
	 * @param ip The IP address of the server.
	 * @param port The port of the server.
	 */
	ClientModel(const string& ip, int port);
	/**
	 * Destructor of the class.
	 */
	~ClientModel();

	/**
	 * Send a message to the server.
	 * @param message The message to send.
	 */
	void send_message(const string& message);
	/**
	 * Receive a message from the server.
	 * @return The message received.
	 */
	string receive_message();
	/**
	 * Check if the username contains spaces.
	 * @param  username The username to check.
	 * @return True if the username contains spaces, false otherwise.
	 */
	bool contain_spaces(const string& username);
	/**
	 * Get the socket.
	 * @return The socket.
	 */
	int get_sockfd();
	/**
	 * Get the color associated with a user.
	 * @param  user The user's name.
	 * @return The color associated with the user.
	 */
	int get_user_color(const string& user);

private:
	/**
	 * Generates a random number for user colors.
	 * @return A random number between 0 and 255.
	 */
	int generate_random_value();
	string ip;
    int port;
    int sockfd;
    const int BUFFER_SIZE = 1024;
    unordered_map<string, int> user_color_map;
};

#endif