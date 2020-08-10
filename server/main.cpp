#include <iostream>

#include "Listener.h"
#include "configuration.h"

int main() {

    // Load the configuration file
    if(!configuration::load_config_file("backupserver.conf")){
        return EXIT_FAILURE;
    }

    if(!configuration::prepare_environment()){
        return EXIT_FAILURE;
    }

    // The io_context is required for all I/O (including network)
    net::io_context ioc{configuration::nthreads};

    // Create and launch a Listener on the configuration port and address
    std::make_shared<Listener>(ioc,tcp::endpoint{configuration::address, configuration::port})->run();


    // Capture SIGINT and SIGTERM
    // This code allow to perform a clean shutdown of the server (using a signal or ctrl+C)
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&ioc](beast::error_code const& ec, int sign)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                std::stringstream ss;
                ss << "Exit after signal with code " << sign << std::endl;
                std::cout << ss.str();
                ioc.stop();
            });


    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(configuration::nthreads - 1);
    for (auto i = configuration::nthreads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc] {
                    ioc.run();
                });
    ioc.run();


    //if run() returned -> a signal was sent (exit)
    // Block until all the threads exit
    for(auto& t : v)
        t.join();

    return 0;
}
