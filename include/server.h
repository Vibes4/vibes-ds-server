#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <string>
#include <iostream>
using namespace std;

class HTTPServer {
    private:
        int port;
        SOCKET server_socket;
    
    public:
        HTTPServer(int port);
        ~HTTPServer();
        void start();
        void handle_client(SOCKET client_socket);
};
    
#endif