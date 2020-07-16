//
// Created by stealbi on 16/07/20.
//

#include "backup.h"

std::string BASE_PATH;

std::optional<std::string> get_file_digest(std::string path, std::string user) {

    //TODO

    path = user+"/"+path;

    if(path == "user0/fileprova.txt"){
        return "a883dafc480d466ee04e0d6da986bd78eb1fdd2178d04693723da3a8f95d42f4";
    }
    if(path == "user0/fileprova2"){
        return "4355a46b19d348dc2f57c046f8ef63d4538ebb936000f3c9ee954a27460dd865";
    }

    return {};
}

bool save_file(const std::string &filename, const std::string &user, std::unique_ptr<char[]> &&raw_file, std::size_t n) {
    std::string path = BASE_PATH + user + "/" + filename;
    std::ofstream file(path, std::ios::out|std::ios::binary|std::ios::trunc);
    if(file.is_open()){
        file.write(raw_file.get(),n);
        file.close();
        return true;
    } else
        return false;
}
