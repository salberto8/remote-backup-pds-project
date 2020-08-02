//
// Created by giacomo on 31/07/20.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <filesystem>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


// implementare le varie richieste HTTP


class Client {
public:
    Client(beast::tcp_stream &stream);

    bool probe_directory(std::string path);
    void new_directory(std::string path);

private:
    boost::beast::tcp_stream &stream;
};


#endif //CLIENT_CLIENT_H
