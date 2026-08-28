#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "app/controllers/health_controller.h"
#include "app/controllers/redis_controller.h"
#include "http/router.h"
#include "net/tcp_server.h"
#include "redis/command_executor.h"

// The application server. It ties the layers together: TcpServer accepts
// connections, each request is parsed as HTTP, the Router dispatches it to a
// controller, and the controller's response is written back. All protocol and
// command logic lives in the layers below; this class is just the wiring root:
// it owns the executor and the controllers and registers their routes.
class HttpServer {
public:
    explicit HttpServer(int port);
    void start();

private:
    void handle_connection(SocketType client_socket);

    int port_;
    TcpServer tcp_;
    Router router_;
    CommandExecutor executor_;
    RedisController redis_controller_;
    HealthController health_controller_;
};

#endif  // SERVER_SERVER_H
