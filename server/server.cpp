//
// Created by stealbi on 16/07/20.
//

#include "server.h"


void fail(beast::error_code ec, const std::string &what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

void replaceSpaces(std::string &str) {
    int pos;

    while ((pos=str.find("%20"))!= std::string::npos){
        str.replace(pos, 3, " ");
    }

}