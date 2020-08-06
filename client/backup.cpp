//
// Created by giacomo on 05/08/20.
//

#include <openssl/evp.h>
#include <boost/beast/core/detail/base64.hpp>
#include <memory>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>

#include "backup.h"

namespace base64 = boost::beast::detail::base64;


#define BUF_SIZE 2048

std::string calculate_digest(std::string path) {
    EVP_MD_CTX *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    char hex_digest[EVP_MAX_MD_SIZE*2+1];
    int n,i;
    unsigned int md_len;
    unsigned char buf[BUF_SIZE];
    FILE * fin;

    if((fin = fopen(path.c_str(),"rb")) == NULL) {
        return {};
    }

    md = EVP_MD_CTX_new();
    EVP_MD_CTX_init(md);

    EVP_DigestInit(md, EVP_sha256());

    while((n = fread(buf,1,BUF_SIZE,fin)) > 0)
        EVP_DigestUpdate(md, buf,n);

    if(EVP_DigestFinal_ex(md, md_value, &md_len) != 1) {
        //error computing the digest
        return {};
    }

    EVP_MD_CTX_free(md);

    //convert in hex
    for(i = 0; i < md_len; i++)
        sprintf(hex_digest+2*i,"%02x", md_value[i]);

    hex_digest[md_len*2] = 0;

    std::string digest(hex_digest);

    return digest;
}

std::unique_ptr<char[]> encode(const std::string &path) {
    std::size_t original_len = std::filesystem::file_size(path);
    std::size_t encoded_len = base64::encoded_size(original_len);

    std::unique_ptr<char[]> encoded_file{new char[encoded_len + 1]};
    std::unique_ptr<char[]> original_file{new char[original_len]};

    std::ifstream file (path, std::ios::in | std::ios::binary);
    file.read (original_file.get(), original_len);

    std::size_t real_len = base64::encode(encoded_file.get(), original_file.get(), original_len);
    // the resulting string is not null terminated
    encoded_file[real_len] = 0;

    //std::cout<< encoded_file.get()<<std::endl;
    return std::move(encoded_file);
}

std::set<std::string> get_children(const std::string &path) {
    std::set<std::string> set;
    for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
        set.insert(entry.path().filename().string());
    }
    return std::move(set);
}