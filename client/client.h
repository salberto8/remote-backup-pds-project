//
// Created by giacomo on 03/08/20.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;       // from <boost/beast/http.hpp>


bool probe_file(const std::string& original_path);
bool backup_file(const std::string& original_path);
bool probe_folder(const std::string& original_path);
bool backup_folder(const std::string& original_path);
bool delete_path(const std::string& original_path);
bool authenticateToServer();

#endif //CLIENT_CLIENT_H
