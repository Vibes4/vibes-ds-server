#include "server.h"
#include "helper.h"
#include "kv_store.h"
KVStore kvStore;

HTTPServer::HTTPServer(int port) : port(port)
{
#if IS_WINDOWS
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        cerr << "Error creating socket\n";
        exit(1);
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cerr << "Bind failed\n";
        exit(1);
    }

    if (listen(server_socket, 5) < 0)
    {
        cerr << "Listen failed\n";
        exit(1);
    }
}

HTTPServer::~HTTPServer()
{
#if IS_WINDOWS
    closesocket(server_socket);
    WSACleanup();
#else
    close(server_socket);
    exit(0);
#endif
}

void HTTPServer::start()
{
    cout << "Server started on port " << port << "\n";
    while (true)
    {
        SocketType client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0)
        {
            cout << "Accept failed\n";
            cerr << "Accept failed\n";
            continue;
        }

        handle_client(client_socket);
        #if IS_WINDOWS
            closesocket(client_socket);
        #else 
            close(client_socket);
        #endif
    }
}

void HTTPServer::handle_client(SocketType client_socket)
{
    char buffer[1024] = {0};
    recv(client_socket, buffer, sizeof(buffer), 0);
    // Print the full HTTP request to console
    string request(buffer);

    string response;

    // Check if request starts with /redis
    if (request.find("GET /redis/store/key-value-pair-td") == 0)
    {
        // Get all key-value pairs from the store and add them to the HTML
        string html = "";
        for (const auto &pair : kvStore.getAll())
        {
            html += "<tr><td>" + pair.first + "</td><td>" + pair.second + "</td>";
            html += "<td><button onclick='editKey(\"" + pair.first + "\")'>Edit</button>";
            html += "<button onclick='deleteKey(\"" + pair.first + "\")'>Delete</button></td></tr>";
        }
        send_response(client_socket, "200 OK", html, "text/html");
    }
    else if (request.find("GET /redis/store") == 0)
    {
        string html_content = read_file("template/store.html");
        send_response(client_socket, "200 OK", html_content, "text/html");
    }
    else if (request.find("GET /redis") == 0)
    {
        // Parse query parameters (cmd, key, value)
        auto params = parse_query(request);
        string cmd = params["cmd"];
        string key = params["key"];
        string value = params["value"];
        // Process Redis commands
        if (cmd == "SET" && !key.empty() && !value.empty())
        {
            kvStore.set(key, value);
            send_response(client_socket, "200 OK", "OK");
        }
        else if (cmd == "GET" && !key.empty())
        {
            send_response(client_socket, "200 OK", kvStore.get(key));
        }
        else if (cmd == "DEL" && !key.empty())
        {
            kvStore.del(key);
            send_response(client_socket, "200 OK", "Deleted");
        }
        else
        {
            send_response(client_socket, "400 Bad Request", "Invalid Command");
        }
    }
    else if (request.find("GET /ping") == 0)
    {
        send_response(client_socket, "200 OK", getAboutProject());
    }
    else if (request.find("GET /about") == 0)
    {
        string html_content = read_file("template/about.html");
        send_response(client_socket, "200 OK", html_content, "text/html");
    }

    send_response(client_socket, "200 OK", "Hello, World!");
}