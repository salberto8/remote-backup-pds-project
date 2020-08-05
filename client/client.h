//
// Created by giacomo on 03/08/20.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <boost/beast/http.hpp>

//#include <mutex>
//#include <condition_variable>

namespace http = boost::beast::http;       // from <boost/beast/http.hpp>

void handle_response(http::response<http::string_body> *res);

bool probe_file(const std::string& original_path);
bool backup_file(const std::string& original_path);


#endif //CLIENT_CLIENT_H
