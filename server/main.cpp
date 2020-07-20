#include <iostream>

#include "Listener.h"
#include "configuration.h"
#include "dao.h"

int main() {

    const std::string conf_path = "/home/stealbi/Desktop/testpds/backupserver.conf";

    if(!configuration::load_config_file(conf_path)){
        return -1;
    }





    // The io_context is required for all I/O
    net::io_context ioc{configuration::nthreads};

    // Create and launch a listening port
    std::make_shared<Listener>(ioc,tcp::endpoint{configuration::address, configuration::port})->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(configuration::nthreads - 1);
    for (auto i = configuration::nthreads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc] {
                    ioc.run();
                });
    ioc.run();

    return 0;
}
