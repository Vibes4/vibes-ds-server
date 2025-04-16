#include "helper.h"

std::map<std::string, std::string> parse_query(const std::string& request) {
    std::map<std::string, std::string> params;
    std::istringstream request_stream(request);
    std::string request_line;

    if (!std::getline(request_stream, request_line)) {
        return params;
    }

    // Extract only the part after "GET " or "POST " and before " HTTP/1.1"
    size_t first_space = request_line.find(" ");
    size_t second_space = request_line.find(" ", first_space + 1);
    if (first_space == std::string::npos || second_space == std::string::npos) {
        return params;  // Malformed request
    }

    std::string path_and_query = request_line.substr(first_space + 1, second_space - first_space - 1);

    size_t pos = path_and_query.find("?");
    if (pos == std::string::npos) return params;  // No query string

    std::string query_string = path_and_query.substr(pos + 1);
    std::istringstream query_stream(query_string);
    std::string pair;

    while (std::getline(query_stream, pair, '&')) {
        size_t eq_pos = pair.find("=");
        if (eq_pos != std::string::npos) {
            std::string key = pair.substr(0, eq_pos);
            std::string value = pair.substr(eq_pos + 1);
            params[key] = value;
        }
    }

    return params;
}

void send_response(SocketType client_socket, const std::string& status, const std::string& body, const std::string& content_type) {
    std::string response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: " + content_type + "; charset=UTF-8\r\n"
        "Content-Length: " + std::to_string(body.length()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;

    size_t total_sent = 0;
    size_t response_length = response.length();
    const char* response_data = response.c_str();

    while (total_sent < response_length) {
        int sent = send(client_socket, response_data + total_sent, response_length - total_sent, 0);
        if (sent < 0) {
#ifdef PLATFORM_WINDOWS
            std::cerr << "Error sending response: " << WSAGetLastError() << std::endl;
#else
            std::perror("Error sending response");
#endif
            return;
        }
        total_sent += sent;
    }
}

std::string getAboutProject() {
    return
        "Welcome to the Vibes-Data-Structure Server!\n\n"
        "This server implements a lightweight, Redis-like key-value store over HTTP.\n"
        "It supports basic commands such as:\n"
        "  - SET <key> <value>  → Store a value\n"
        "  - GET <key>          → Retrieve a value\n"
        "  - DEL <key>          → Delete a key\n"
        "  - PING               → Check if the server is running\n\n"
        "Features:\n"
        "✔ Uses sockets for efficient communication\n"
        "✔ Thread-safe with mutex for concurrent requests\n"
        "✔ Can handle multiple clients at once\n\n"
        "Developed by: Vaibhav (SDE)\n";
}

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "File not found: " << filename << std::endl;
        return "<h1>404 Not Found</h1><p>File not found: " + filename + "</p>";
    }

    std::ostringstream content_stream;
    content_stream << file.rdbuf();
    return content_stream.str();
}
