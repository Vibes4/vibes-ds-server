#ifndef HTTP_HTTP_REQUEST_H
#define HTTP_HTTP_REQUEST_H

#include <map>
#include <string>

// A parsed HTTP request.
//
// Only the request line (e.g. "GET /redis?cmd=GET&key=x HTTP/1.1") is
// inspected, which is all this server currently needs. Headers and body are
// ignored.
struct HttpRequest {
    std::string method;                        // "GET", "POST", ...
    std::string path;                          // path without the query string
    std::map<std::string, std::string> query;  // decoded query parameters

    // Parses the raw request buffer into an HttpRequest.
    static HttpRequest parse(const std::string& raw);

    // Returns the query parameter for `key`, or an empty string if absent.
    std::string param(const std::string& key) const;
};

#endif  // HTTP_HTTP_REQUEST_H
