#ifndef SERVER_H
#define SERVER_H

#include "platform.h"
#include <string>

class HTTPServer {
    private:
        int port;
        SocketType server_socket;
    
    public:
        HTTPServer(int port);
        ~HTTPServer();
        void start();
        void handle_client(SocketType client_socket);
        void handle_redis_command(SocketType client_socket, const std::string& request);
        void handle_key_value_table(SocketType client_socket);
        std::string extract_path(const std::string& request);
};
    
#endif