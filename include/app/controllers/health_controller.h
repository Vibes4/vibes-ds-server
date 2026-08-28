#ifndef APP_CONTROLLERS_HEALTH_CONTROLLER_H
#define APP_CONTROLLERS_HEALTH_CONTROLLER_H

#include "http/router.h"

// Controller for the /ping endpoint.
//
// Serves a plain-text liveness/info response describing the server and how to
// send commands. It has no dependencies on the storage or command layers.
class HealthController
{
public:
    // Registers this controller's routes on the given router.
    void register_routes(Router &router);

private:
    HttpResponse handle_ping(const HttpRequest &request);
};

#endif  // APP_CONTROLLERS_HEALTH_CONTROLLER_H
