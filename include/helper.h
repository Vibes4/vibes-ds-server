#ifndef HELPER_H
#define HELPER_H

#include "platform.h"
#include <string>
#include <map>
#include <sstream>
#include <fstream>

std::map<std::string, std::string> parse_query(const std::string& request);
void send_response(SocketType client_socket, const std::string& status, const std::string& body, const std::string& content_type = "text/plain");
std::string getAboutProject();
std::string read_file(const std::string& filename); 

#endif  // HELPER_H
