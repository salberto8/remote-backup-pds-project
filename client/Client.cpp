//
// Created by giacomo on 31/07/20.
//

#include "Client.h"


Client::Client(beast::tcp_stream &stream) : stream(stream) {

}

bool Client::probe_directory(std::string path) {
    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, "/probefolder/"+path, 11};
    req.set(http::field::authorization, "aaa");
    //req.set(http::field::host, "127.0.0.1:12345");
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    if(res.result() == http::status::ok){
        return true;
    }

    return false;
}

void Client::new_directory(std::string path) {

}

