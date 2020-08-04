//
// Created by giacomo on 31/07/20.
//

#include <iostream>
#include <pwd.h>
#include <fstream>
#include <boost/program_options.hpp>

#include "configuration.h"


namespace po = boost::program_options;

namespace configuration
{
    net::ip::address address;
    short unsigned port;
    std::string backup_path;
}



bool configuration::load_config_file(const std::string &config_file) {
    std::string home_dir;
    if ((getenv("HOME")) != NULL)
        home_dir = (getenv("HOME"));
    else
        home_dir = getpwuid(getuid())->pw_dir;

    const std::string conf_path = home_dir + "/" + config_file;


    std::ifstream conf_file {conf_path};

    if(!conf_file){
        std::cerr << "Can't open the configuration file" << std::endl;
        return false;
    }

    po::options_description desc("Allowed options");
    desc.add_options()
            ("address", "host address")
            ("port", po::value<unsigned short>(), "host port")
            ("backup_path", "path where you want the backup done")
            ;

    po::variables_map vm;
    po::store(po::parse_config_file(conf_file, desc), vm);
    //po::notify(vm);

    try {
        configuration::address = net::ip::make_address(vm["address"].as<std::string>());
        configuration::port = vm["port"].as<unsigned short>();
        configuration::backup_path = vm["backup_path"].as<std::string>();
    } catch(boost::bad_any_cast & e){
        std::cerr << "Bad configuration file, usage:\n" << desc << std::endl;
        return false;
    }
    return true;
}