#include <iostream>
#include <pwd.h>
#include <unistd.h>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "configuration.h"
#include "FileWatcher.h"
#include "Client.h"


namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

int main() {
    std::string home_dir;
    if ((getenv("HOME")) != NULL)
        home_dir = (getenv("HOME"));
    else
        home_dir = getpwuid(getuid())->pw_dir;

    const std::string conf_path = home_dir + "/backup.conf";

    if(!configuration::load_config_file(conf_path)){
        return -1;
    }

    // FileWatcher refer to a path with a time interval at which we check for changes
    FileWatcher fw{configuration::backup_path, std::chrono::milliseconds(5000)};

    fw.start();



    // da finire
    try
    {
        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Make the connection on the IP address
        stream.connect(tcp::endpoint( configuration::address, configuration::port));

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, "http://127.0.0.1:12345/probefolder/prova", 1.1};
        req.set(http::field::authorization, "aaa");
        req.set(http::field::host, "127.0.0.1:12345");
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;
        
        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Write the message to standard out
        std::cout << res << std::endl;

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    return 0;
}
