#ifndef APP_ROUTES_H
#define APP_ROUTES_H

#include "http/router.h"
#include "redis/redis_service.h"

// Registers every HTTP endpoint the server exposes on `router`, wiring the
// Redis-backed endpoints to `redis`. This is the map of the application: the
// full list of URLs and what each one does lives in one place.
void register_routes(Router& router, RedisService& redis);

#endif  // APP_ROUTES_H
