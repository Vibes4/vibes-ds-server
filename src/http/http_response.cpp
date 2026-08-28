#include "http/http_response.h"

#include "observability/logger.h"

#include <utility>

HttpResponse::HttpResponse(std::string status, std::string body,
                           std::string content_type)
    : status(std::move(status)),
      body(std::move(body)),
      content_type(std::move(content_type)) {}

HttpResponse HttpResponse::ok(const std::string &body,
                              const std::string &content_type)
{
    return HttpResponse("200 OK", body, content_type);
}

HttpResponse HttpResponse::html(const std::string &body)
{
    return HttpResponse("200 OK", body, "text/html");
}

HttpResponse HttpResponse::not_found(const std::string &message)
{
    return HttpResponse("404 Not Found", message);
}

HttpResponse HttpResponse::bad_request(const std::string &message)
{
    return HttpResponse("400 Bad Request", message);
}

void write_response(SocketType client_socket, const HttpResponse &response)
{
    const std::string payload =
        "HTTP/1.1 " + response.status + "\r\n"
                                        "Content-Type: " +
        response.content_type + "; charset=UTF-8\r\n"
                                "Content-Length: " +
        std::to_string(response.body.size()) + "\r\n"
                                               "Connection: close\r\n"
                                               "\r\n" +
        response.body;

    // send() may transmit fewer bytes than requested, so loop until done.
    size_t total_sent = 0;
    while (total_sent < payload.size())
    {
        const int sent = send(client_socket, payload.c_str() + total_sent,
                              payload.size() - total_sent, 0);
        if (sent < 0)
        {
            Logger::error("http", "failed to send response to client");
            return;
        }
        total_sent += static_cast<size_t>(sent);
    }
}
