//
// Created by stealbi on 14/07/20.
//

#ifndef SERVER_PROGETTO_BACKUP_H
#define SERVER_PROGETTO_BACKUP_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <fstream>


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


extern std::string BASE_PATH;

std::optional<std::string> get_file_digest(std::string path, std::string user);


bool save_file(const std::string &filename,const std::string &user,std::unique_ptr<char []> &&raw_file,std::size_t n);


#endif //SERVER_PROGETTO_BACKUP_H