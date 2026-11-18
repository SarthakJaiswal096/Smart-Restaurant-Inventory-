#ifndef MINI_HTTPLIB_H
#define MINI_HTTPLIB_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <exception>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifdef _MSC_VER
    #pragma comment(lib, "ws2_32.lib")
    #endif
    typedef int socklen_compat;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <signal.h>
    typedef int SOCKET;
    #ifndef INVALID_SOCKET
    #define INVALID_SOCKET (-1)
    #endif
    #define closesocket close
    typedef socklen_t socklen_compat;
#endif

using namespace std;

namespace httplib
{
    struct Request
    {
        string method;
        string path;
        string body;
        map<string, string> params;
        map<string, string> headers;

        bool has_param(const string& key) const
        {
            return params.find(key) != params.end();
        }

        string get_param_value(const string& key) const
        {
            auto it = params.find(key);
            return (it != params.end()) ? it->second : "";
        }

        bool has_header(const string& key) const
        {
            return headers.find(key) != headers.end();
        }
    };

    struct Response
    {
        int status = 200;
        string body;
        string content_type = "text/plain";
        map<string, string> headers;

        void set_content(const string& b, const string& type)
        {
            body = b;
            content_type = type;
        }

        void set_header(const string& key, const string& value)
        {
            headers[key] = value;
        }
    };

    using Handler = function<void(const Request&, Response&)>;

    class Server
    {
    private:
        string mount_url;
        string mount_dir;
        map<string, Handler> get_routes;
        map<string, Handler> post_routes;

        static int hexValue(char c)
        {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        }

        // Converts %20 etc. in URL parameters back to normal characters.
        // A broken escape such as "%zz" is kept as plain text (no exception).
        string urlDecode(const string& s)
        {
            string out;
            for (size_t i = 0; i < s.size(); i++)
            {
                if (s[i] == '%' && i + 2 < s.size() && hexValue(s[i + 1]) >= 0 && hexValue(s[i + 2]) >= 0)
                {
                    out += (char)(hexValue(s[i + 1]) * 16 + hexValue(s[i + 2]));
                    i += 2;
                }
                else if (s[i] == '+') out += ' ';
                else out += s[i];
            }
            return out;
        }

        void parseQuery(const string& query, map<string, string>& params)
        {
            stringstream ss(query);
            string pair;
            while (getline(ss, pair, '&'))
            {
                size_t eq = pair.find('=');
                if (eq != string::npos)
                    params[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
                else if (!pair.empty())
                    params[urlDecode(pair)] = "";
            }
        }

        static bool endsWith(const string& s, const string& suffix)
        {
            return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        string mimeType(const string& file)
        {
            if (endsWith(file, ".html") || endsWith(file, ".htm")) return "text/html; charset=utf-8";
            if (endsWith(file, ".css")) return "text/css; charset=utf-8";
            if (endsWith(file, ".js")) return "application/javascript; charset=utf-8";
            if (endsWith(file, ".json")) return "application/json";
            if (endsWith(file, ".svg")) return "image/svg+xml";
            if (endsWith(file, ".png")) return "image/png";
            if (endsWith(file, ".jpg") || endsWith(file, ".jpeg")) return "image/jpeg";
            if (endsWith(file, ".ico")) return "image/x-icon";
            return "text/plain; charset=utf-8";
        }

        static const char* statusText(int code)
        {
            switch (code)
            {
                case 200: return "OK";
                case 204: return "No Content";
                case 400: return "Bad Request";
                case 403: return "Forbidden";
                case 404: return "Not Found";
                case 405: return "Method Not Allowed";
                case 500: return "Internal Server Error";
                default:  return "OK";
            }
        }

        static string jsonError(const string& message)
        {
            string safe;
            for (size_t i = 0; i < message.size(); i++)
            {
                char c = message[i];
                if (c == '"' || c == '\\') safe += '\\';
                if (c == '\n' || c == '\r') c = ' ';
                safe += c;
            }
            return "{\"ok\":false,\"error\":\"" + safe + "\"}";
        }

        // Reads one complete request (headers + body) from the socket.
        bool readRequest(SOCKET client, string& raw)
        {
            const size_t MAX_REQUEST = 1024 * 1024;
            char buf[4096];
            size_t headerEnd = string::npos;
            size_t needed = 0;

            while (true)
            {
                if (headerEnd == string::npos)
                {
                    headerEnd = raw.find("\r\n\r\n");
                    if (headerEnd != string::npos)
                    {
                        headerEnd += 4;
                        // find Content-Length (case-insensitive)
                        string head = raw.substr(0, headerEnd);
                        for (size_t i = 0; i < head.size(); i++)
                            if (head[i] >= 'A' && head[i] <= 'Z') head[i] = (char)(head[i] - 'A' + 'a');
                        size_t cl = head.find("content-length:");
                        size_t len = 0;
                        if (cl != string::npos) len = (size_t)strtoul(head.c_str() + cl + 15, NULL, 10);
                        needed = headerEnd + len;
                    }
                }
                if (headerEnd != string::npos && raw.size() >= needed) return true;
                if (raw.size() > MAX_REQUEST) return false;

                int n = (int)recv(client, buf, sizeof(buf), 0);
                if (n <= 0) return headerEnd != string::npos; // closed or timed out
                raw.append(buf, (size_t)n);
            }
        }

        bool parseRequest(const string& raw, Request& req)
        {
            size_t lineEnd = raw.find("\r\n");
            if (lineEnd == string::npos) return false;

            stringstream first(raw.substr(0, lineEnd));
            string full_url, version;
            first >> req.method >> full_url >> version;
            if (req.method.empty() || full_url.empty()) return false;

            size_t qpos = full_url.find('?');
            if (qpos != string::npos)
            {
                req.path = urlDecode(full_url.substr(0, qpos));
                parseQuery(full_url.substr(qpos + 1), req.params);
            }
            else req.path = urlDecode(full_url);

            size_t pos = lineEnd + 2;
            size_t headerEnd = raw.find("\r\n\r\n");
            while (headerEnd != string::npos && pos < headerEnd)
            {
                size_t e = raw.find("\r\n", pos);
                string line = raw.substr(pos, e - pos);
                size_t colon = line.find(':');
                if (colon != string::npos)
                {
                    string key = line.substr(0, colon), value = line.substr(colon + 1);
                    for (size_t i = 0; i < key.size(); i++)
                        if (key[i] >= 'A' && key[i] <= 'Z') key[i] = (char)(key[i] - 'A' + 'a');
                    size_t a = value.find_first_not_of(" \t");
                    req.headers[key] = (a == string::npos) ? "" : value.substr(a);
                }
                pos = e + 2;
            }
            if (headerEnd != string::npos) req.body = raw.substr(headerEnd + 4);

            // A form body (name=a&id=2) is treated like extra query parameters.
            if (req.method == "POST" && !req.body.empty() &&
                req.headers["content-type"].find("application/x-www-form-urlencoded") != string::npos)
                parseQuery(req.body, req.params);
            return true;
        }

        void serveFile(const Request& req, Response& res)
        {
            string file = (req.path == "/") ? "/index.html" : req.path;
            if (mount_url.size() > 1)
            {
                string prefix = mount_url;
                if (prefix[prefix.size() - 1] == '/') prefix.erase(prefix.size() - 1);
                if (file.compare(0, prefix.size(), prefix) != 0)
                {
                    res.status = 404;
                    res.set_content("404 Not Found", "text/plain");
                    return;
                }
                file = file.substr(prefix.size());
                if (file.empty() || file == "/") file = "/index.html";
            }

            if (file.find("..") != string::npos || file.find('\\') != string::npos)
            {
                res.status = 403;
                res.set_content("403 Forbidden", "text/plain");
                return;
            }
            ifstream in((mount_dir + file).c_str(), ios::binary);
            if (in)
            {
                stringstream content;
                content << in.rdbuf();
                res.set_content(content.str(), mimeType(file));
            }
            else
            {
                res.status = 404;
                res.set_content("404 Not Found", "text/plain");
            }
        }

        void route(const Request& req, Response& res)
        {
            if (req.method == "OPTIONS")
            {
                res.status = 204;
                return;
            }
            if (req.method == "GET" && get_routes.count(req.path))
                get_routes[req.path](req, res);
            else if (req.method == "POST" && post_routes.count(req.path))
                post_routes[req.path](req, res);
            else if ((req.method == "GET" || req.method == "HEAD") && req.path.compare(0, 5, "/api/") != 0 && !mount_dir.empty())
                serveFile(req, res);
            else if (req.path.compare(0, 5, "/api/") == 0)
            {
                bool known = get_routes.count(req.path) || post_routes.count(req.path);
                res.status = known ? 405 : 404;
                res.set_content(jsonError(known ? "Wrong method for this route" : "Unknown API route"), "application/json");
            }
            else
            {
                res.status = 404;
                res.set_content("404 Not Found", "text/plain");
            }
        }

        bool sendAll(SOCKET client, const string& data)
        {
            size_t sent = 0;
            while (sent < data.size())
            {
#ifdef MSG_NOSIGNAL
                int n = (int)send(client, data.c_str() + sent, (int)(data.size() - sent), MSG_NOSIGNAL);
#else
                int n = (int)send(client, data.c_str() + sent, (int)(data.size() - sent), 0);
#endif
                if (n <= 0) return false;
                sent += (size_t)n;
            }
            return true;
        }

        void handleClient(SOCKET client)
        {
            string raw;
            Request req;
            Response res;

            if (!readRequest(client, raw)) return;       // idle / broken connection
            if (!parseRequest(raw, req))
            {
                res.status = 400;
                res.set_content(jsonError("Malformed request"), "application/json");
            }
            else
            {
                try { route(req, res); }
                catch (const exception& e)
                {
                    res.status = 400;
                    res.set_content(jsonError(string("Bad request value: ") + e.what()), "application/json");
                }
                catch (...)
                {
                    res.status = 500;
                    res.set_content(jsonError("Internal server error"), "application/json");
                }
            }

            string header = "HTTP/1.1 " + to_string(res.status) + " " + statusText(res.status) + "\r\n"
                          + "Content-Type: " + res.content_type + "\r\n"
                          + "Content-Length: " + to_string(req.method == "HEAD" ? 0 : res.body.size()) + "\r\n"
                          + "Access-Control-Allow-Origin: *\r\n"
                          + "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                          + "Access-Control-Allow-Headers: Content-Type\r\n"
                          + "Cache-Control: no-store\r\n";
            for (map<string, string>::iterator it = res.headers.begin(); it != res.headers.end(); ++it)
                header += it->first + ": " + it->second + "\r\n";
            header += "Connection: close\r\n\r\n";

            sendAll(client, req.method == "HEAD" ? header : header + res.body);
        }

    public:
        void set_mount_point(const string& url, const string& dir)
        {
            mount_url = url;
            mount_dir = dir;
            while (!mount_dir.empty() && (mount_dir[mount_dir.size() - 1] == '/' || mount_dir[mount_dir.size() - 1] == '\\'))
                mount_dir.erase(mount_dir.size() - 1);
        }

        void Get(const string& path, Handler h) { get_routes[path] = h; }
        void Post(const string& path, Handler h) { post_routes[path] = h; }

        bool listen(const string& host, int port)
        {
#ifdef _WIN32
            WSADATA wsa;
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            {
                cerr << "WSAStartup failed.\n";
                return false;
            }
#else
            signal(SIGPIPE, SIG_IGN); // a closed browser tab must not kill the server
#endif
            SOCKET server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == (SOCKET)INVALID_SOCKET)
            {
                cerr << "Could not create socket.\n";
                return false;
            }

            int opt = 1;
            ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

            sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = (host.empty() || host == "0.0.0.0") ? htonl(INADDR_ANY) : inet_addr(host.c_str());
            addr.sin_port = htons((unsigned short)port);

            if (::bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
            {
                cerr << "Could not bind to port " << port << " (is the server already running?).\n";
                closesocket(server_fd);
                return false;
            }
            if (::listen(server_fd, 16) < 0)
            {
                cerr << "listen() failed.\n";
                closesocket(server_fd);
                return false;
            }

            while (true)
            {
                SOCKET client = ::accept(server_fd, NULL, NULL);
                if (client == (SOCKET)INVALID_SOCKET) continue;

                // Browsers open idle "pre-connect" sockets. Without a timeout the
                // single-threaded server would wait on them forever and the page
                // would appear to hang.
#ifdef _WIN32
                DWORD timeout = 1000;
                ::setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
                struct timeval tv;
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                ::setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
                handleClient(client);
                closesocket(client);
            }
            return true;
        }
    };
}

#endif
