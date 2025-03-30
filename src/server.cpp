#include "server.h"

HTTPServer::HTTPServer(int port): port(port) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        cerr << "Error creating socket\n";
        exit(1);
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Bind failed\n";
        exit(1);
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        cerr << "Listen failed\n";
        exit(1);
    }
}

HTTPServer::~HTTPServer() {
    closesocket(server_socket);
    WSACleanup();
}

void HTTPServer::start(){
    cout << "Server started on port " << port << "\n";
    while (true) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            cout << "Accept failed\n";
            cerr << "Accept failed\n";
            continue;
        }

        handle_client(client_socket);
        closesocket(client_socket);
    }
}

void HTTPServer::handle_client(SOCKET client_socket) {
    char buffer[1024] = {0};
    recv(client_socket, buffer, sizeof(buffer), 0);
    string response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello, World!";
    send(client_socket, response.c_str(), response.length(), 0);
}