#include "helper.h"

map<string, string> parse_query(const string& request) {
    map<string, string> params;
    istringstream request_stream(request);
    string request_line;

    if (!getline(request_stream, request_line)) {
        return params;
    }

    // Extract only the part after "GET " or "POST " and before " HTTP/1.1"
    size_t first_space = request_line.find(" ");
    size_t second_space = request_line.find(" ", first_space + 1);
    if (first_space == string::npos || second_space == string::npos) {
        return params;  // Malformed request
    }

    string path_and_query = request_line.substr(first_space + 1, second_space - first_space - 1);

    // Find the '?' in the extracted path
    size_t pos = path_and_query.find("?");
    if (pos == string::npos) return params;  // No query parameters

    // Extract the query string
    string query_string = path_and_query.substr(pos + 1);
    istringstream query_stream(query_string);
    string pair;
    
    // Parse key-value pairs
    while (getline(query_stream, pair, '&')) {
        size_t eq_pos = pair.find("=");
        if (eq_pos != string::npos) {
            params[pair.substr(0, eq_pos)] = pair.substr(eq_pos + 1);
        }
    }
    return params;
}



// Utility to send HTTP responses
void send_response(SocketType client_socket, const string& status, const string& body, const string& content_type) {
    string response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type:"+ content_type + ";charset=UTF-8\r\n"
        "Content-Length: " + to_string(body.length()) + "\r\n"
        "Connection: close\r\n"  // Ensures client knows the connection will close
        "\r\n" + body;

    size_t total_sent = 0;
    size_t response_length = response.length();
    const char* response_data = response.c_str();

    while (total_sent < response_length) {
        int sent = send(client_socket, response_data + total_sent, response_length - total_sent, 0);
        if (sent < 0) {
            cerr << "Error sending response: " << 
            #if IS_WIDNOWS 
                WSAGetLastError()  
            #else 
                "" 
            #endif 
            << endl;
            return;
        }
        total_sent += sent;
    }
}



string getAboutProject() {
    return "Welcome to the Vibes-Data-Structure Server!\n"
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


string read_file(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "File not found: " << filename << endl;
        return "<h1>404 Not Found</h1><p>File not found: " + filename + "</p>";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return content;
}