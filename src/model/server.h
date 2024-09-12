#ifndef SERVER_MODEL_H
#define SERVER_MODEL_H

#include <mutex>
#include <unordered_map>
#include <string>
#include <sys/socket.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

class ServerModel {
public:
	bool add_user(const string& client_id, int client_sockfd);
	void remove_user(const string& client_id);
	void message_everyone(const json& message, const string& user);
	void message_private(const json& message, const string& user);

private:
	mutex mtx;
    unordered_map<string, int> user_socket_map;
};

#endif