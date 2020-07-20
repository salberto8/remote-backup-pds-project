//
// Created by stealbi on 15/07/20.
//

#include "authorization.h"




std::optional<std::string> verifyToken(const std::string &token) {
    Dao *dao = Dao::getInstance();

    std::optional<std::string> opt_user = dao->getUserFromToken(token);

    return opt_user;
};