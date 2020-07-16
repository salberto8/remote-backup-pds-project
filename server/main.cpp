#include <iostream>
#include <boost/program_options.hpp>

#include "Listener.h"


namespace po = boost::program_options;


int main() {


    const std::string conf_path = "/home/stealbi/Desktop/testpds/backupserver.conf";
    std::ifstream conf_file {conf_path};

    if(!conf_file){
        std::cerr << "Can't open the configuration file" << std::endl;
        return -1;
    }

    po::options_description desc("Allowed options");
    desc.add_options()
            ("address", "listen socket address")
            ("port", po::value<unsigned short>(), "listen socket port")
            ("nthreads", po::value<int>(), "number of threads")
            ("keyfile", "file with the server secret")
            ("backuppath", "path where backups are stored")
            ;

    po::variables_map vm;
    po::store(po::parse_config_file(conf_file, desc), vm);
    //po::notify(vm);


    net::ip::address address;
    unsigned short port;
    int threads;


    try {
        address = net::ip::make_address(vm["address"].as<std::string>());
        port = vm["port"].as<unsigned short>();
        threads = std::max<int>(1, vm["nthreads"].as<int>());
        BASE_PATH = vm["backuppath"].as<std::string>();
        std::string key_file = vm["keyfile"].as<std::string>();
        if(!initialize_server_key(key_file)){
            std::cerr << "Key file not found" << std::endl;
            return -1;
        }

    } catch(boost::bad_any_cast & e){
        std::cerr << "Bad configuration file, usage:\n" << desc << std::endl;
        return -1;
    }





    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<Listener>(ioc,tcp::endpoint{address, port})->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc] {
                    ioc.run();
                });
    ioc.run();

    return 0;
}
