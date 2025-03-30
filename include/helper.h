#ifndef HELPER_H
#define HELPER_H

#include <string>
#include <map>
#include <winsock2.h>

using namespace std;

// Function declarations (only declarations, no definitions here)
map<string, string> parse_query(const string& request);
void send_response(SOCKET client_socket, const string& status, const string& body);
string getAboutProject();

#endif  // HELPER_H
