//
// Created by giacomo on 03/08/20.
//

#include <iostream>
#include <thread>
#include <openssl/evp.h>
#include <cstdlib>

#include "client.h"
#include "configuration.h"
#include "Session.h"

std::mutex m;
std::condition_variable cv;
http::status http_status = http::status::unknown;
std::string remote_digest;


void replaceSpaces(std::string &str) {

    // Getting the length of the string, counting the number of spaces
    int strLen = str.length();
    int i, count = 0;
    for (i = 0; i <= strLen; i++) {
        if(str[i]==' ')
            count++;
    }

    // Determining the new length needed to allocate for replacement characters '%20'
    int newLength = strLen + count * 2;

    str.resize(newLength);
    for (i = strLen - 1; i >= 0; i--) {
        if (str[i] == ' ') {
            str[newLength - 1] = '0';
            str[newLength - 2] = '2';
            str[newLength - 3] = '%';
            newLength = newLength - 3;
        }

        else {
            str[newLength - 1] = str[i];
            newLength = newLength -1;
        }
    }
}

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

bool probe_file(std::string path) {
    std::string original_path = path;
    // make the relative path
    path.erase(0, configuration::backup_path.length());

    // substitute spaces with %20
    replaceSpaces(path);
    char tmp[100] = "/probefile";
    if(path.length() > 80) {
        std::cerr << "path" << path << "too long" << std::endl;
    }
    strcat(tmp, path.c_str());
    const char* target_path = tmp;

    net::io_context ioc;

    // Launch the asynchronous operation
    std::make_shared<Session>(ioc)->run(configuration::address.c_str(), configuration::port.c_str(), target_path, 11);

    // Run the I/O service. The call will return when the get operation is complete.
    std::thread t_probe_file(
            [&ioc] {
                ioc.run();
            });
    t_probe_file.detach();

    std::string local_digest = calculate_digest(original_path);

    std::unique_lock<std::mutex> l(m);
    cv.wait(l, [](){
        return (http_status != http::status::unknown);
    });
    if(http_status == http::status::ok)
        return true;

    return false;
}

void backup(std::string path) {
    // make the relative path
    path.erase(0, configuration::backup_path.length());

    // substitute spaces with %20
    replaceSpaces(path);
    char tmp[100] = "/backup";
    if(path.length() > 80) {
        std::cerr << "path" << path << "too long" << std::endl;
    }
    strcat(tmp, path.c_str());
    const char* target_path = tmp;

    net::io_context ioc;

    // Launch the asynchronous operation
    std::make_shared<Session>(ioc)->run(configuration::address.c_str(), configuration::port.c_str(), target_path, 11);

    // Run the I/O service. The call will return when the get operation is complete.
    ioc.run();

    if(http_status != http::status::ok) {
        std::cerr << "Error: server cannot create the folder '" << path << "'" << std::endl;
    }
}