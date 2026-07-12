#ifndef NET_TCP_SERVER_H
#define NET_TCP_SERVER_H

#include "platform/platform.h"
#include <functional>

// A minimal, cross-platform TCP listener.
//
// It owns the listening socket and, for every accepted connection, spawns a
// detached thread that hands the client socket to the supplied handler. The
// handler does not need to close the socket; TcpServer does that once it
// returns. TcpServer knows nothing about HTTP, so it can back any protocol.
class TcpServer {
public:
    using ConnectionHandler = std::function<void(SocketType)>;

    explicit TcpServer(int port);
    ~TcpServer();

    // Blocks forever, accepting connections and dispatching each to `handler`.
    void run(const ConnectionHandler& handler);

private:
    SocketType listen_socket_;
};

#endif  // NET_TCP_SERVER_H
