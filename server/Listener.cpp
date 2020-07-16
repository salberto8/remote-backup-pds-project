//
// Created by stealbi on 14/07/20.
//

#include "Listener.h"

void Listener::do_accept() {
    // The new connection gets its own strand
    acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                    &Listener::on_accept,
                    shared_from_this()));
}

void Listener::on_accept(beast::error_code ec, tcp::socket socket) {
    if(ec){
        fail(ec, "accept");
    }
    else{
        // Create the session and run it
        std::make_shared<Session>(std::move(socket))->run();
    }

    // Accept another connection
    do_accept();
}
