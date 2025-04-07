#include "server.h"
#include "helper.h"
#include "kv_store.h"
#include <thread>

KVStore kvStore;

HTTPServer::HTTPServer(int port) : port(port) {
#ifdef PLATFORM_WINDOWS
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed.\n";
        exit(1);
    }
#endif

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket.\n";
        exit(1);
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed.\n";
        exit(1);
    }

    if (listen(server_socket, 5) < 0) {
        std::cerr << "Listen failed.\n";
        exit(1);
    }
}

HTTPServer::~HTTPServer() {
#ifdef PLATFORM_WINDOWS
    closesocket(server_socket);
    WSACleanup();
#else
    close(server_socket);
#endif
}

void HTTPServer::start() {
    std::cout << "Server started on port " << port << "\n";

    while (true) {
        SocketType client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        std::thread(&HTTPServer::handle_client, this, client_socket).detach();
    }
}

void HTTPServer::handle_client(SocketType client_socket) {
    char buffer[2048] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
#ifdef PLATFORM_WINDOWS
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }

    std::string request(buffer);
    std::string path = extract_path(request);

    if (path.find("/redis/store/key-value-pair-td") == 0) {
        handle_key_value_table(client_socket);
    }
    else if (path.find("/redis/store") == 0) {
        std::string html = read_file("template/store.html");
        send_response(client_socket, "200 OK", html, "text/html");
    }
    else if (path.find("/redis") == 0) {
        handle_redis_command(client_socket, request);
    }
    else if (path.find("/ping") == 0) {
        send_response(client_socket, "200 OK", getAboutProject());
    }
    else if (path.find("/about") == 0) {
        std::string html = read_file("template/about.html");
        send_response(client_socket, "200 OK", html, "text/html");
    }
    else {
        send_response(client_socket, "404 Not Found", "Invalid endpoint.");
    }

#ifdef PLATFORM_WINDOWS
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

std::string HTTPServer::extract_path(const std::string& request) {
    std::istringstream stream(request);
    std::string method, path;
    stream >> method >> path;
    return path;
}

void HTTPServer::handle_key_value_table(SocketType client_socket) {
    std::string html;
    for (const auto& [key, value] : kvStore.getAll()) {
        html += "<tr><td>" + key + "</td><td>" + value + "</td>";
        html += "<td><button onclick='editKey(\"" + key + "\")'>Edit</button>";
        html += "<button onclick='deleteKey(\"" + key + "\")'>Delete</button></td></tr>";
    }
    send_response(client_socket, "200 OK", html, "text/html");
}

void HTTPServer::handle_redis_command(SocketType client_socket, const std::string& request) {
    auto params = parse_query(request);
    std::string cmd = params["cmd"];
    std::string key = params["key"];
    std::string value = params["value"];

    if (cmd == "SET" && !key.empty() && !value.empty()) {
        kvStore.set(key, value);
        send_response(client_socket, "200 OK", "OK");
    }
    else if (cmd == "GET" && !key.empty()) {
        send_response(client_socket, "200 OK", kvStore.get(key));
    }
    else if (cmd == "DEL" && !key.empty()) {
        kvStore.del(key);
        send_response(client_socket, "200 OK", "Deleted");
    }
    else {
        send_response(client_socket, "400 Bad Request", "Invalid Command");
    }
}
