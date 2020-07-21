//
// Created by stealbi on 16/07/20.
//

#include "backup.h"
#include "configuration.h"

#include <openssl/evp.h>



#define BUF_SIZE 1024


std::optional<std::string> get_file_digest(std::string path, std::string user) {

    path = configuration::backuppath + user + "/" + path;


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



    if(path == "user0/fileprova.txt"){
        return "a883dafc480d466ee04e0d6da986bd78eb1fdd2178d04693723da3a8f95d42f4";
    }
    if(path == "user0/fileprova2"){
        return "4355a46b19d348dc2f57c046f8ef63d4538ebb936000f3c9ee954a27460dd865";
    }

    return {};
}

bool save_file(const std::string &filename, const std::string &user, std::unique_ptr<char[]> &&raw_file, std::size_t n) {
    std::string path = configuration::backuppath + user + "/" + filename;
    std::ofstream file(path, std::ios::out|std::ios::binary|std::ios::trunc);
    if(file.is_open()){
        file.write(raw_file.get(),n);
        file.close();
        return true;
    } else
        return false;
}
