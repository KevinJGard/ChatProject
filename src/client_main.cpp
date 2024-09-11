#include "controller/client_controller.h"
#include <iostream>
#include <cstdlib>
using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <IP> <port>" << endl;
        exit(EXIT_FAILURE);
    }

    string ip = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        cerr << "Invalid port number. Port must be between 1 and 65535." << endl;
        exit(EXIT_FAILURE);
    }
    ClientController client(ip, port);
    client.run();

    return 0;
}