//
// Created by giacomo on 04/08/20.
//

#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

// Performs an HTTP request and saves the response
class Session : public std::enable_shared_from_this<Session>
{
    tcp::resolver resolver_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::string_body>& req_; // received by reference
    http::response<http::string_body>& res_; // received by reference

public:
    // Objects are constructed with a strand to ensure that handlers do not execute concurrently.
    explicit
    Session(net::io_context& ioc,
            http::request<http::string_body>& req,
            http::response<http::string_body>& res);

    // Start the asynchronous operation
    void
    run();

    void
    on_resolve(
            beast::error_code ec,
            const tcp::resolver::results_type& results);

    void
    on_connect(beast::error_code ec, const tcp::resolver::results_type::endpoint_type&);

    void
    on_write(
            beast::error_code ec,
            std::size_t bytes_transferred);

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred);
};


#endif //CLIENT_SESSION_H
