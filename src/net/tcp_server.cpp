#include "net/tcp_server.h"

#include <cstdlib>
#include <thread>

TcpServer::TcpServer(int port) {
#ifdef PLATFORM_WINDOWS
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed.\n";
        std::exit(1);
    }
#endif

    listen_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket_ < 0) {
        std::cerr << "Error creating socket.\n";
        std::exit(1);
    }

    // Allow the server to rebind immediately after a restart instead of waiting
    // out the previous socket's TIME_WAIT period.
    int reuse = 1;
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(listen_socket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed.\n";
        std::exit(1);
    }

    if (listen(listen_socket_, 5) < 0) {
        std::cerr << "Listen failed.\n";
        std::exit(1);
    }
}

TcpServer::~TcpServer() {
    close_socket(listen_socket_);
#ifdef PLATFORM_WINDOWS
    WSACleanup();
#endif
}

void TcpServer::run(const ConnectionHandler& handler) {
    while (true) {
        SocketType client = accept(listen_socket_, nullptr, nullptr);
        if (client < 0) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        std::thread([handler, client]() {
            handler(client);
            close_socket(client);
        }).detach();
    }
}
