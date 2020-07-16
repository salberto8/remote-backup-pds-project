//
// Created by stealbi on 16/07/20.
//

#include "server.h"


void fail(beast::error_code ec, const std::string &what) {
    std::cerr << what << ": " << ec.message() << "\n";
}
