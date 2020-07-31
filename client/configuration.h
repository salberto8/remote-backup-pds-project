//
// Created by giacomo on 31/07/20.
//

#ifndef CLIENT_CONFIGURATION_H
#define CLIENT_CONFIGURATION_H


#include <string>
#include <boost/asio.hpp>

namespace net = boost::asio;

namespace configuration
{
    extern net::ip::address address;
    extern short unsigned port;
    extern std::string backup_path;

    bool load_config_file(const std::string &conf_path);
}


#endif //CLIENT_CONFIGURATION_H
