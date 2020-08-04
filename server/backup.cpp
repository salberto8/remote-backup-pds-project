//
// Created by stealbi on 16/07/20.
//

#include "backup.h"
#include "configuration.h"

#include <openssl/evp.h>
#include <filesystem>

namespace fs = std::filesystem;


#define BUF_SIZE 2048


bool save_file(const std::string &user, const std::string &filename, std::unique_ptr<char[]> &&raw_file, std::size_t n) {

    std::string path = configuration::backuppath + user + "/" + filename;

    std::ofstream file(path, std::ios::out|std::ios::binary|std::ios::trunc);
    if(file.is_open()){
        file.write(raw_file.get(),n);
        file.close();
        return true;
    } else
        return false;
}


std::optional<std::string> get_file_digest(const std::string &user, const std::string& file_path) {

    std::string path = configuration::backuppath + user + "/" + file_path;

    if(!fs::is_regular_file(path))
        return {};

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


bool new_directory(const std::string& user, const std::string& path){
    return fs::create_directory(configuration::backuppath + user + "/" + path);
}




bool probe_directory(const std::string& user, const std::string& path, const std::set<std::string> & children){
    std::string abs_path = configuration::backuppath + user+"/"+path;
    if(!fs::is_directory(abs_path))
        return false;

    fs::directory_iterator end_itr;
    for (fs::directory_iterator itr(abs_path); itr!=end_itr; ++itr){
        const std::string filename = itr->path().filename();
        const std::string file_path = itr->path();

        //check if the file is still present in the client
        if(children.count(filename)==0){
            //remove file/directory
            if (fs::is_directory(file_path)) {
                fs::remove_all(file_path); //recursive elimination!
            } else {
                fs::remove(file_path);
            }
        }
    }

    return true;
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
