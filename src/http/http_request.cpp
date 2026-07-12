#include "http/http_request.h"

#include <sstream>

HttpRequest HttpRequest::parse(const std::string &raw)
{
    HttpRequest request;

    // The request line is "<method> <target> HTTP/<version>".
    std::istringstream stream(raw);
    std::string target;
    stream >> request.method >> target;

    const size_t query_start = target.find('?');
    request.path = target.substr(0, query_start);
    if (query_start == std::string::npos)
    {
        return request; // no query string
    }

    // Split the query string into "key=value" pairs separated by '&'.
    std::istringstream query_stream(target.substr(query_start + 1));
    std::string pair;
    while (std::getline(query_stream, pair, '&'))
    {
        const size_t eq = pair.find('=');
        if (eq != std::string::npos)
        {
            request.query[pair.substr(0, eq)] = pair.substr(eq + 1);
        }
    }

    return request;
}

std::string HttpRequest::param(const std::string &key) const
{
    const auto it = query.find(key);
    return it != query.end() ? it->second : "";
}
