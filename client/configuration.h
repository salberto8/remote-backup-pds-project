//
// Created by giacomo on 31/07/20.
//

#ifndef CLIENT_CONFIGURATION_H
#define CLIENT_CONFIGURATION_H


#include <string>
//#include <boost/asio.hpp>

//namespace net = boost::asio;

namespace configuration
{
    extern std::string address;
    extern std::string port;
    extern std::string backup_path;
    extern std::string username;
    extern std::string token;

    bool load_config_file(const std::string &config_file);
}


#endif //CLIENT_CONFIGURATION_H
