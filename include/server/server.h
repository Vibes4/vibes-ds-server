#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "http/router.h"
#include "net/tcp_server.h"
#include "redis/redis_service.h"

// The application server. It ties the layers together: TcpServer accepts
// connections, each request is parsed as HTTP, the Router dispatches it to a
// handler, and the handler's response is written back. All protocol and
// command logic lives in the layers below; this class is just the wiring.
class HttpServer {
public:
    explicit HttpServer(int port);
    void start();

private:
    void handle_connection(SocketType client_socket);

    int port_;
    TcpServer tcp_;
    Router router_;
    RedisService redis_;
};

#endif  // SERVER_SERVER_H
