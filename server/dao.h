#ifndef SERVER_DAO_H
#define SERVER_DAO_H

#include <sqlite3.h>
#include <iostream>
#include <future>
#include <mutex>
#include <thread>

#include "configuration.h"

//singleton, interface with database
class Dao{
    Dao();
    ~Dao();

    sqlite3* db;
    bool conn_open;

public:
    static Dao* instance;
    static std::once_flag inited;

    static Dao *getInstance();

    std::optional<std::string> getUserFromToken(const std::string &token);
    std::optional<std::string> getPasswordFromUser(const std::string &username);
    bool insertTokenToUser(const std::string &username, const std::string &token);
    bool deleteTokenToUser(const std::string &username);
    std::vector<std::string> getAllUsers();
    void deleteAllTokens();

    Dao(const Dao&)= delete;
    Dao& operator=(const Dao&)= delete;
};

#endif //SERVER_DAO_H