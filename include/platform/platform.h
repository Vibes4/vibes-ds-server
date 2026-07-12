#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H

// Cross-platform socket layer.
// Windows uses Winsock (SOCKET/closesocket); POSIX uses BSD sockets (int/close).
// The rest of the codebase depends only on the abstractions defined here.

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketType;
#else
    #define PLATFORM_UNIX
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SocketType;
#endif

#include <iostream>

// Closes a socket regardless of platform.
inline void close_socket(SocketType socket) {
#ifdef PLATFORM_WINDOWS
    closesocket(socket);
#else
    close(socket);
#endif
}

#endif  // PLATFORM_PLATFORM_H
