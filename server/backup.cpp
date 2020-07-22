//
// Created by stealbi on 16/07/20.
//

#include "backup.h"
#include "configuration.h"

#include <openssl/evp.h>
#include <boost/filesystem.hpp>


namespace fs = boost::filesystem;



#define BUF_SIZE 2048


std::optional<std::string> get_file_digest(const std::string& file_path, const std::string &user) {

    std::string path = configuration::backuppath + user + "/" + file_path;


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

bool new_directory(const std::string& user, const std::string& path){
    return fs::create_directory(configuration::backuppath + user+"/"+path);
}

bool probe_directory(const std::string& user, const std::string& path){
    return fs::is_directory(configuration::backuppath + user+"/"+path);
}

bool backup_delete(const std::string& user, const std::string& path){
    std::string abs_path = configuration::backuppath + user+"/"+path;

    if (fs::is_directory(abs_path)){
        return fs::remove_all(abs_path) > 0; //recursive elimination!
    }
    if (fs::is_regular_file(abs_path)){
        return fs::remove(abs_path);
    }
    return false;
}