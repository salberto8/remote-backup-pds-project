//
// Created by stealbi on 15/07/20.
//

#ifndef SERVER_PROGETTO_AUTHORIZATION_H
#define SERVER_PROGETTO_AUTHORIZATION_H



#include <jwt-cpp/jwt.h>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>

//initialize the key reading from the given file
bool initialize_server_key(const std::string &path);

// return the username if authorized, {} otherwise
std::optional<std::string> verifyJwt(const std::string& token);



#endif //SERVER_PROGETTO_AUTHORIZATION_H