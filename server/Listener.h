#ifndef SERVER_PROGETTO_LISTENER_H
#define SERVER_PROGETTO_LISTENER_H

#include "Session.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener> {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    Listener(net::io_context& ioc, const tcp::endpoint &endpoint);

    void run();

private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};


#endif //SERVER_PROGETTO_LISTENER_H