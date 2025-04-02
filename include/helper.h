#ifndef HELPER_H
#define HELPER_H

#include <string>
#include <map>

#if IS_WINDOWS
#include <winsock2.h>
typedef SOCKET SocketType;
#else
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
typedef int SocketType;
#endif

#include <iostream>
#include <map>    
#include <sstream>    
#include <fstream>

using namespace std;

// Function declarations (only declarations, no definitions here)
map<string, string> parse_query(const string& request);
void send_response(SocketType client_socket, const string& status, const string& body, const string& content_type = "text/plain");
string getAboutProject();
string read_file(const string& filename); 

#endif  // HELPER_H
