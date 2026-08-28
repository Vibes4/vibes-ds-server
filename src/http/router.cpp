#include "http/router.h"

#include <utility>

void Router::add(const std::string& prefix, Handler handler) {
    routes_.push_back({prefix, std::move(handler)});
}

HttpResponse Router::route(const HttpRequest& request) const {
    for (const auto& route : routes_) {
        // rfind(prefix, 0) == 0 is true iff path starts with prefix.
        if (request.path.rfind(route.prefix, 0) == 0) {
            return route.handler(request);
        }
    }
    return HttpResponse::not_found("Invalid endpoint.");
}
