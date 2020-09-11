//
// Created by giacomo on 31/07/20.
//

#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include "configuration.h"


namespace po = boost::program_options;

namespace configuration
{
    std::string address;
    std::string port;
    std::string backup_path;
    std::string username;
    std::string token;
}


bool configuration::load_config_file(const std::string &config_file)
{
    // load the home path of the Windows or Unix user
    #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    const std::string home_dir = (getenv("HOMEPATH"));
    #else
    const std::string home_dir = (getenv("HOME"));
    #endif

    const std::string conf_path = home_dir + "/" + config_file;

    std::ifstream conf_file {conf_path};

    if(!conf_file){
        std::cerr << "Can't open the configuration file" << std::endl;
        return false;
    }

    po::options_description desc("Allowed options");
    desc.add_options()
            ("address", "host address")
            ("port", "host port")
            ("backup_path", "path where you want the backup done")
            ("username", "username for authentication to the server")
            ;

    po::variables_map vm;
    po::store(po::parse_config_file(conf_file, desc), vm);
    //po::notify(vm);

    try {
        configuration::address = vm["address"].as<std::string>();
        configuration::port = vm["port"].as<std::string>();
        configuration::backup_path = vm["backup_path"].as<std::string>();
        configuration::username = vm["username"].as<std::string>();
        configuration::token = "";

        char end_slash = 47; // "/"
        if(configuration::backup_path.back() != end_slash) {
            configuration::backup_path = configuration::backup_path + end_slash;
        }
    } catch(boost::bad_any_cast & e){
        std::cerr << "Bad configuration file, usage:\n" << desc << std::endl;
        return false;
    }
    return true;
}