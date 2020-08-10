//
// Created by giacomo on 03/08/20.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;       // from <boost/beast/http.hpp>


bool probe_file(const std::string& original_path);
void backup_file(const std::string& original_path);
bool probe_folder(const std::string& original_path);
void backup_folder(const std::string& original_path);
void delete_path(const std::string& original_path);
bool authenticateToServer();
bool logout();


#endif //CLIENT_CLIENT_H
