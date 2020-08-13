//
// Created by stealbi on 14/07/20.
//

#ifndef SERVER_PROGETTO_SESSION_H
#define SERVER_PROGETTO_SESSION_H

#include "server.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>



// Handles an HTTP server connection
class Session : public std::enable_shared_from_this<Session>
{

    // The function object is used to send an HTTP message.
    class send_lambda{
        Session& self_;
    public:
        explicit send_lambda(Session& self): self_(self){}

        template<bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg) const {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write(
                    self_.stream_,
                    *sp,
                    beast::bind_front_handler(
                            &Session::on_write,
                            self_.shared_from_this(),
                            sp->need_eof()));
        }
    };


    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::request_parser<http::string_body> parser;
    std::shared_ptr<void> res_;
    send_lambda lambda_;

public:
    // Take ownership of the stream
    explicit Session(
            tcp::socket&& socket)
            : stream_(std::move(socket))
            , lambda_(*this)
    {
        // Allow for an unlimited body size
        parser.body_limit((std::numeric_limits<std::uint64_t>::max)());
    }

    // Start the asynchronous operation
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.

        net::dispatch(stream_.get_executor(),
                      beast::bind_front_handler(
                              &Session::do_read,
                              shared_from_this()));
    }

    void
    do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};


        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(60));



        // Read a request
        /*http::async_read(stream_, buffer_, req_,
                         beast::bind_front_handler(
                                 &Session::on_read,
                                 shared_from_this()));
        */
        http::async_read(stream_, buffer_, parser,
                         beast::bind_front_handler(
                                 &Session::on_read,
                                 shared_from_this()));
    }

    void
    on_read(
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        req_ = parser.get();

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");

        // Send the response
        handle_request(std::move(req_), lambda_);
    }

    void
    on_write(
            bool close,
            beast::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        return do_close();

        // We're done with the response so delete it
        res_ = nullptr;

        // Read another request
        do_read();
    }

    void
    do_close()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};



#endif //SERVER_PROGETTO_SESSION_H