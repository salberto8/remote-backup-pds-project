//
// Created by stealbi on 15/07/20.
//

#ifndef SERVER_PROGETTO_AUTHORIZATION_H
#define SERVER_PROGETTO_AUTHORIZATION_H



#include <fstream>
#include <sstream>
#include <string>
#include <optional>

#include "dao.h"


// return the username if is a valid token, {} otherwise
std::optional<std::string> verifyToken(const std::string& token);

#endif //SERVER_PROGETTO_AUTHORIZATION_H