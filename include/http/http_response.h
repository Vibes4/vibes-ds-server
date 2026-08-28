#ifndef HTTP_HTTP_RESPONSE_H
#define HTTP_HTTP_RESPONSE_H

#include "platform/platform.h"
#include <string>

// A minimal HTTP response: status line, body and content type.
//
// Handlers build one of these and return it; the server serialises it to the
// wire with write_response(). The named constructors cover the cases this
// server actually produces.
struct HttpResponse {
    std::string status;        // e.g. "200 OK", "404 Not Found"
    std::string body;
    std::string content_type;  // e.g. "text/plain", "text/html"

    HttpResponse(std::string status, std::string body,
                 std::string content_type = "text/plain");

    static HttpResponse ok(const std::string& body,
                           const std::string& content_type = "text/plain");
    static HttpResponse html(const std::string& body);
    static HttpResponse not_found(const std::string& message);
    static HttpResponse bad_request(const std::string& message);
};

// Serialises `response` and writes every byte to the client socket.
void write_response(SocketType client_socket, const HttpResponse& response);

#endif  // HTTP_HTTP_RESPONSE_H
