//
// Created by giacomo on 03/08/20.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <filesystem>
#include <mutex>
#include <condition_variable>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

extern std::mutex m;
extern std::condition_variable cv;
extern http::status http_status;
extern std::string remote_digest;

// implementare le varie richieste HTTP

bool probe_directory(std::string path);
bool probe_file(std::string path);
void backup(std::string path);


#endif //CLIENT_CLIENT_H
