#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32
#define IS_WINDOWS true
#else
#define IS_WINDOWS false
#endif

#if IS_WINDOWS
#include <winsock2.h>
typedef SOCKET SocketType;
#else
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
typedef int SocketType;
#endif

#include <string>
#include <iostream>
#include <unistd.h>

using namespace std;

class HTTPServer {
    private:
        int port;
        #if IS_WINDOWS
            SOCKET server_socket;
        #else
            int server_socket;
        #endif
    
    public:
        HTTPServer(int port);
        ~HTTPServer();
        void start();
        void handle_client(SocketType client_socket);
};
    
#endif