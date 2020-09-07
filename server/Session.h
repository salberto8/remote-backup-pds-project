#ifndef SERVER_PROGETTO_SESSION_H
#define SERVER_PROGETTO_SESSION_H

#include "server.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Handles an HTTP server connection
class Session : public std::enable_shared_from_this<Session>{

    // The function object is used to send an HTTP message.
    class SendLambda{
        Session& self_;
    public:
        explicit SendLambda(Session& self): self_(self){}

        template<bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg) const {
            // The lifetime of the message has to extend for the duration of
            // the async operation so we use a shared_ptr to manage it.
            auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store the shared pointer in the class to keep it alive with the Session object.
            self_.res_ = sp;

            // Write the response
            http::async_write(self_.stream_, *sp,
                    beast::bind_front_handler(&Session::on_write, self_.shared_from_this(), sp->need_eof()));
        }
    };

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::request_parser<http::string_body> parser;
    std::shared_ptr<void> res_;
    SendLambda lambda_;

public:

    explicit Session(tcp::socket&& socket);
    ~Session();

    void run();
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred);
};

#endif //SERVER_PROGETTO_SESSION_H