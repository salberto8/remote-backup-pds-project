//
// Created by stealbi on 20/07/20.
//

#ifndef SERVER_CONFIGURATION_H
#define SERVER_CONFIGURATION_H

#include <string>
#include <boost/asio.hpp>

namespace net = boost::asio;

namespace configuration
{

    extern net::ip::address address;
    extern short unsigned port;
    extern int nthreads;
    extern std::string backuppath;
    extern std::string dbpath;

    bool load_config_file(const std::string &config_file);
}



#endif //SERVER_CONFIGURATION_H
