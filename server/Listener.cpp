#include "Listener.h"

/**
 * Constructor, create and set the listening socket
 */
Listener::Listener(net::io_context &ioc, const tcp::endpoint &endpoint)
        : ioc_(ioc), acceptor_(net::make_strand(ioc))
{
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec){
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if(ec){
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec){
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if(ec){
        fail(ec, "listen");
        return;
    }
}

/**
 * start the listener
 */
void Listener::run() {
    // Start accepting incoming connections
    do_accept();
}

/**
 * listen for a new connection
 */
void Listener::do_accept() {
    // The new connection gets its own strand (for serial execution inside the context)
    acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(&Listener::on_accept,shared_from_this()));
}


/**
 * used as callback, when the socket receive a new connection
 * at the end call again do_accept and wait for new connections
 *
 * @param ec error code
 * @param socket new socket created
 */
void Listener::on_accept(beast::error_code ec, tcp::socket socket) {
    if(ec){
        fail(ec, "accept");
    } else {
        // Create the session and run it
        std::make_shared<Session>(std::move(socket))->run();
    }

    // Accept another connection
    do_accept();
}



