//
// Created by stealbi on 20/07/20.
//

#include <fstream>
#include <iostream>
#include <boost/program_options.hpp>
#include <pwd.h>

#include "configuration.h"


namespace po = boost::program_options;



namespace configuration
{
    //definition
    net::ip::address address;
    short unsigned port;
    int nthreads;
    std::string backuppath;
    std::string dbpath;
}

bool configuration::load_config_file(const std::string &config_file)
{
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
            ("address", "listen socket address")
            ("port", po::value<unsigned short>(), "listen socket port")
            ("nthreads", po::value<int>(), "number of threads")
            ("backuppath", "path where backups are stored")
            ("dbpath", "path of the database file")
            ;

    po::variables_map vm;
    po::store(po::parse_config_file(conf_file, desc), vm);
    //po::notify(vm);



    try {
        configuration::address = net::ip::make_address(vm["address"].as<std::string>());
        configuration::port = vm["port"].as<unsigned short>();
        configuration::nthreads = std::max<int>(1, vm["nthreads"].as<int>());
        configuration::backuppath = vm["backuppath"].as<std::string>();
        configuration::dbpath = vm["dbpath"].as<std::string>();
    } catch(boost::bad_any_cast & e){
        std::cerr << "Bad configuration file, usage:\n" << desc << std::endl;
        return false;
    }
    return true;
}