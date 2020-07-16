//
// Created by stealbi on 15/07/20.
//

#include "authorization.h"



std::string secret;


bool initialize_server_key(const std::string &path){
    std::ifstream file(path);
    if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        secret = buffer.str();
        return true;
    }
    return false;
}

std::optional<std::string> verifyJwt(const std::string &token) {

    //if the secret is not initialized block verification
    if(secret.empty())
        return {};

    auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ secret });



    try {
        auto decoded = jwt::decode(token);
        verifier.verify(decoded);
        std::string user = decoded.get_payload_claim("user").as_string();
        return user;
    } catch ( std::runtime_error & e){
        return {};
    }
};