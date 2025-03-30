#include "server.h"
#include "helper.h"
#include "kv_store.h"
KVStore kvStore;

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
    // Print the full HTTP request to console
    string request(buffer);

    cout << "Received Request:\n" << request << "\n-------------------\n";
    string response;

    // Check if request starts with /redis
    if (request.find("GET /redis") != 0) {
        // Parse query parameters (cmd, key, value)
        auto params = parse_query(request);
        string cmd = params["cmd"];
        string key = params["key"];
        string value = params["value"];
            // Process Redis commands
        if (cmd == "SET" && !key.empty() && !value.empty()) {
            kvStore.set(key, value);
            send_response(client_socket, "200 OK", "OK");
        } else if (cmd == "GET" && !key.empty()) {
            send_response(client_socket, "200 OK", kvStore.get(key));
        } else if (cmd == "DEL" && !key.empty()) {
            kvStore.del(key);
            send_response(client_socket, "200 OK", "Deleted");
        } else {
            send_response(client_socket, "400 Bad Request", "Invalid Command");
        }
    } else if(request.find("GET /ping") != 0){
        send_response(client_socket, "200 OK", getAboutProject());
    }

    send_response(client_socket, "200 OK", "Hello, World!");
}