#include <iostream>
#include <string>      
#include <map>        
#include <sstream>    
#include <winsock2.h>
#include "helper.h"
using namespace std;

map<string, string> parse_query(const string& request) {
    map<string, string> params;
    size_t pos = request.find("?");
    if (pos == string::npos) return params;

    istringstream query(request.substr(pos + 1));
    string pair;
    while (getline(query, pair, '&')) {
        size_t eq_pos = pair.find("=");
        if (eq_pos != string::npos) {
            params[pair.substr(0, eq_pos)] = pair.substr(eq_pos + 1);
        }
    }
    return params;
}

// Utility to send HTTP responses
void send_response(SOCKET client_socket, const string& status, const string& body) {
    string response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + to_string(body.length()) + "\r\n"
        "\r\n" + body;
    send(client_socket, response.c_str(), response.length(), 0);
}


string getAboutProject() {
    return "Welcome to the Vibes-DS Server!\n"
           "\n"
           "This server implements a lightweight, Redis-like key-value store over HTTP.\n"
           "It supports basic commands such as:\n"
           "  - SET <key> <value>  → Store a value\n"
           "  - GET <key>          → Retrieve a value\n"
           "  - DEL <key>          → Delete a key\n"
           "  - PING               → Check if the server is running\n"
           "\n"
           "Features:\n"
           "✔ Uses sockets for efficient communication\n"
           "✔ Thread-safe with mutex for concurrent requests\n"
           "✔ Can handle multiple clients at once\n"
           "\n"
           "Developed by: Vaibhav (SDE)\n";
}
