#include <iostream>

#include "Listener.h"
#include "configuration.h"
#include "dao.h"

int main() {

    // load the config file
    if(!configuration::load_config_file("backupserver.conf")){
        return EXIT_FAILURE;
    }

    // The io_context is required for all I/O
    net::io_context ioc{configuration::nthreads};

    // Create and launch a listening port
    std::make_shared<Listener>(ioc,tcp::endpoint{configuration::address, configuration::port})->run();


    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&](beast::error_code const&, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                std::cout << "Exit after signal" << std::endl;
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


    // Block until all the threads exit
    for(auto& t : v)
        t.join();

    return 0;
}
