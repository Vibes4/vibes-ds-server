#include "server/server.h"

#include "http/http_request.h"
#include "http/http_response.h"

HttpServer::HttpServer(int port)
    : port_(port), tcp_(port), redis_controller_(executor_) {
    redis_controller_.register_routes(router_);
    health_controller_.register_routes(router_);
}

void HttpServer::start() {
    std::cout << "Server started on port " << port_ << "\n";
    tcp_.run([this](SocketType client) { handle_connection(client); });
}

void HttpServer::handle_connection(SocketType client_socket) {
    char buffer[2048] = {0};
    const int received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        return;  // TcpServer closes the socket once this returns
    }

    const HttpRequest request = HttpRequest::parse(std::string(buffer, received));
    const HttpResponse response = router_.route(request);
    write_response(client_socket, response);
}
