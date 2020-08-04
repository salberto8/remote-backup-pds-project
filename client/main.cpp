#include <iostream>

#include "configuration.h"
#include "FileWatcher.h"

int main() {
    // load the config file
    if(!configuration::load_config_file("backup.conf")){
        return EXIT_FAILURE;
    }
    if(!std::filesystem::exists(configuration::backup_path)) {
        std::cerr << configuration::backup_path << " not exists" << std::endl;
        return EXIT_FAILURE;
    }

    // start the connection with the server
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    try {
        stream.connect(tcp::endpoint( configuration::address, configuration::port));
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // FileWatcher refer to a path with a time interval at which we check for changes
    FileWatcher fw{configuration::backup_path, std::chrono::milliseconds(5000), &stream};
    // thread for the FileWatcher
    std::thread thread_fw(&FileWatcher::start, &fw);



    thread_fw.join();

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // maybe this is not needed
    if(ec && ec != beast::errc::not_connected)
        throw beast::system_error{ec};

    return 0;
}
