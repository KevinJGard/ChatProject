#ifndef SERVER_MODEL_H
#define SERVER_MODEL_H

#include <mutex>
#include <unordered_map>
#include <string>
#include <sys/socket.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

/**
 * The model component of the server.
 */
class ServerModel {
public:
	/**
	 * Adds a new user to the server with their socket
	 */
	bool add_user(const string& client_id, int client_sockfd);
	void remove_user(const string& client_id);
	unordered_map<string, string> get_map();
	void change_status(const string& client_id, const string& status);
	bool find_in_map(const string& user);
	void send_message(const json& message, int client_sockfd);
	void send_message_everyone(const json& message, const string& user);
	void send_message_private(const json& message, const string& user);

private:
	mutex mtx;
    unordered_map<string, int> user_socket_map;
    unordered_map<string, string> user_status_map;
};

#endif