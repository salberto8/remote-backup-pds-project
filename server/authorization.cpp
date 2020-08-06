//
// Created by stealbi on 15/07/20.
//

#include "authorization.h"
#include <openssl/crypto.h>
#include <openssl/evp.h>



std::optional<std::string> verifyToken(const std::string &token) {
    Dao *dao = Dao::getInstance();

    std::optional<std::string> opt_user = dao->getUserFromToken(token);

    return opt_user;
};

std::string compute_password_digest(std::string password) {
    EVP_MD_CTX *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    char hex_digest[EVP_MAX_MD_SIZE*2+1];
    int n,i;
    unsigned int md_len;


    md = EVP_MD_CTX_new();
    EVP_MD_CTX_init(md);

    EVP_DigestInit(md, EVP_sha256());

    n = password.length();
    char *pwd = new char [n+1];
    std::strcpy (pwd, password.c_str());

    EVP_DigestUpdate(md, pwd, n);

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

bool verifyUserPassword(const std::string& username, const std::string& password) {

    Dao *dao = Dao::getInstance();

    std::optional<std::string> opt_password = dao->getPasswordFromUser(username);

    if (!opt_password.has_value()){
        return false;
    }
    int p_len = opt_password->length();
    char *pwd_from_db = new char [p_len+1];
    std::strcpy (pwd_from_db, opt_password.value().c_str());


    std::string password_digest = compute_password_digest(password);
    char *pwd = new char [password_digest.length()+1];
    std::strcpy (pwd, password_digest.c_str());

    if (CRYPTO_memcmp(pwd, pwd_from_db, p_len) != 0)
        return false;
    else
        return true;

}

