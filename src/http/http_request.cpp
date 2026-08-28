#include "http/http_request.h"

#include <cctype>
#include <sstream>

namespace
{

// Value of a single hex digit, or -1 if it is not one.
int hex_value(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    return -1;
}

// Decodes application/x-www-form-urlencoded text: '+' -> space and %XX -> byte.
std::string url_decode(const std::string &text)
{
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == '+')
        {
            out += ' ';
        }
        else if (text[i] == '%' && i + 2 < text.size())
        {
            const int hi = hex_value(text[i + 1]);
            const int lo = hex_value(text[i + 2]);
            if (hi >= 0 && lo >= 0)
            {
                out += static_cast<char>(hi * 16 + lo);
                i += 2;
            }
            else
            {
                out += text[i];  // malformed escape, keep the '%'
            }
        }
        else
        {
            out += text[i];
        }
    }
    return out;
}

} // namespace

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
            request.query[url_decode(pair.substr(0, eq))] =
                url_decode(pair.substr(eq + 1));
        }
    }

    return request;
}

std::string HttpRequest::param(const std::string &key) const
{
    const auto it = query.find(key);
    return it != query.end() ? it->second : "";
}
