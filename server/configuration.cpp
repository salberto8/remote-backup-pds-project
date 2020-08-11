//
// Created by stealbi on 20/07/20.
//

#include <fstream>
#include <iostream>
#include <boost/program_options.hpp>
#include <pwd.h>
#include <filesystem>

#include "configuration.h"
#include "dao.h"


namespace po = boost::program_options;
namespace fs = std::filesystem;


namespace configuration
{
    //definition
    net::ip::address address;
    short unsigned port;
    int nthreads;
    std::string backuppath;
    std::string dbpath;
}

/**
 * read a configuration file and save the content in the configuration:: namespace
 *
 * @param config_file path of the configuration file
 * @return false if parsing failed, true otherwise
 */
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

        char end_slash = 47; // "/"
        if(configuration::backuppath.back() != end_slash) {
            configuration::backuppath = configuration::backuppath + end_slash;
        }
    } catch(boost::bad_any_cast & e){
        std::cerr << "Bad configuration file, usage:\n" << desc << std::endl;
        return false;
    }
    return true;
}

/**
 * read all the users present in the database and create a directory for each one (if not already present)
 * in the backuppath
 *
 * @return false if the db returns an empty set of users, true otherwise
 */
bool configuration::prepare_environment(){
    // get dao instance
    Dao *dao = Dao::getInstance();

    std::vector<std::string> users = dao->getAllUsers();

    if (users.empty())
        return false;

    for (std::string user: users){
        std::string path = configuration::backuppath + user;

        if (!fs::is_directory(path)){
            fs::create_directory(path);
        }

    }
    return true;
}