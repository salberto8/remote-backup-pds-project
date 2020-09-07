#include "Session.h"

// Take ownership of the stream
Session::Session(tcp::socket &&socket)
        : stream_(std::move(socket)), lambda_(*this)
{
    // Maximize the body size limit
    parser.body_limit((std::numeric_limits<std::uint64_t>::max)());
}

Session::~Session(){
    // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    // At this point the connection is closed gracefully
}

// Start the asynchronous operation
void Session::run() {
    // The session executes within a strand to perform thread-safe
    // async operations on the I/O objects in this session
    net::dispatch(stream_.get_executor(),
                  beast::bind_front_handler(&Session::do_read,shared_from_this()));
}

void Session::do_read() {
    // Make the request empty before reading,
    req_ = {};
    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(60));

    // Read a request
    http::async_read(stream_, buffer_, parser,
                     beast::bind_front_handler(&Session::on_read,shared_from_this()));
}

void Session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    req_ = parser.get();

    if(ec)
        return fail(ec, "read");

    // Generate and send the response
    handle_request(std::move(req_), lambda_);
}

void Session::on_write(bool close, beast::error_code ec, std::size_t bytes_transferred) {
    if(ec)
        fail(ec, "write");

    // Now the Session object is destroyed (no more shared_pointer)
    // the connection is closed in ~Session
}

