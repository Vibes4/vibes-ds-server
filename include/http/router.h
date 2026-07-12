#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H

#include "http/http_request.h"
#include "http/http_response.h"
#include <functional>
#include <string>
#include <vector>

// Dispatches requests to handlers by matching the start of the request path.
//
// Routes are tested in registration order and the first prefix match wins, so
// register more specific prefixes before more general ones (e.g. "/redis/store"
// before "/redis"). Unmatched requests yield a 404.
class Router {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    void add(const std::string& prefix, Handler handler);
    HttpResponse route(const HttpRequest& request) const;

private:
    struct Route {
        std::string prefix;
        Handler handler;
    };
    std::vector<Route> routes_;
};

#endif  // HTTP_ROUTER_H
